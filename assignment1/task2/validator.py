#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
ParProg/ST19/T1.2/v1

	$ color() (set -o pipefail; "$@" 2>&1>&3 | sed $'s,.*,\e[31m&\e[m,' >&2) 3>&1
	$ make
	$ color python3 path/to/validator.py

"""

import operator
import os
import subprocess
import sys

#-------------------------------------------------------------------------------

Test_cwd = None

output_lines = []

def save_log(msg="", newLine=True, okay=False):
	output_lines.append((okay, "%s\n" % msg if newLine else msg))

def print_log(msg="", newLine=True, okay=False):
	f = sys.stdout if okay else sys.stderr
	f.write("%s\n" % msg if newLine else msg)
	f.flush()

log = save_log

def in_cwd(filename):
	if Test_cwd is None:
		return filename
	return os.path.join(Test_cwd, filename)

def silent_atoi(s):
	try:
		return int(s)
	except ValueError:
		return None

#-------------------------------------------------------------------------------

class Test(object):

	def exec_(self, input_text, *cmd_and_args):
		p = subprocess.Popen(cmd_and_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=Test_cwd)
		return tuple(map(lambda s: s.strip(), p.communicate(input=input_text)[:2])) + (p.returncode, )

	def run_prog(self, *cmd_and_args):
		return self.exec_("", *cmd_and_args)[0]

	def test(self):
		self.run()
		okay = self.okay()
		log(self.what(), okay=okay)
		return okay

	def run(self):
		raise NotImplementedError

	def what(self):
		raise NotImplementedError

	def okay(self):
		raise NotImplementedError


class EmptyTestLine(Test):

	def __init__(self, text=""):
		self.text = text

	def run(self):
		pass

	def what(self):
		return self.text

	def okay(self):
		return True


class TestGroup(Test):
	"""A group of tests, where all tests are executed"""

	def __init__(self, *tests):
		self.tests = tests
		self.is_okay = False
 
	def test(self):
		self.is_okay = all([t.test() for t in self.tests])
		return self.is_okay

	def okay(self):
		return self.is_okay


class AbortingTestGroup(TestGroup):
	"""A group of tests, where the first failed test stops the entire group"""

	def test(self):
		self.is_okay = all(t.test() for t in self.tests)
		return self.is_okay


class ReturnCodeTest(Test):

	def __init__(self, progname, args=None, retcode=0, showstdout=False, showstderr=False, inputText="", truncateLines=None):
		self.progname = progname
		self.inputText = inputText
		self.cmdline = [progname] + list(map(str, args or []))
		self.expected = retcode
		self.exception = None
		self.showstdout = showstdout
		self.showstderr = showstderr
		self.output = None
		self.truncateLines = truncateLines
		self.stdout = ""
		self.stderr = ""

	def _truncate(self, text):
		# Hoping for PEP 572 to fix this mess
		#    https://www.python.org/dev/peps/pep-0572/
		# Okay, hoping for submit-exec to support Python 3.8

		lines = text.split("\n")
		if self.truncateLines is None or len(lines) <= self.truncateLines:
			return text

		return "\n".join(lines[:self.truncateLines] + ["", "[OUTPUT TRUNCATED]"])

	def _get_output(self, tpl):
		self.stdout = tpl[0].decode()
		self.stderr = tpl[1].decode()
		return tpl[2]

	def run(self):
		try:
			self.output = self._get_output(self.exec_(self.inputText, *self.cmdline))
		except Exception as e:
			self.exception = e

	def _s(self, a):
		return " ".join(map(str, a))

	def _formatCall(self):
		if self.inputText:
			return "echo -ne %r | %s" % (self.inputText, self._s(self.cmdline))
		return self._s(self.cmdline)

	def what(self):
		return "$ %s #- exited with %s (expected %d)%s%s" % (
					self._formatCall(),
					str(self.output) if self.exception is None else repr(str(self.exception)),
					self.expected,
					"" if not (self.showstdout and self.stdout) else ("\n%s" % self._truncate(self.stdout)),
					"" if not (self.showstderr and self.stderr) else ("\n%s" % self._truncate(self.stderr)),
				)

	def okay(self):
		return self.output == self.expected


class ExecutionTest(ReturnCodeTest):
	"""Run a programm and compare the lines of stdout to a list of expected output."""

	def __init__(self, progname, args, expected, compare=operator.eq, key=lambda x: x, inputText = "", showstderr=False):
		super(ExecutionTest, self).__init__(progname, args, expected, inputText=inputText, showstderr=showstderr)
		self.compare = compare
		self.key = key

	def _get_output(self, tpl):
		super(ExecutionTest, self)._get_output(tpl)
		return self.stdout.split("\n")

	def what(self):
		_r = repr
		return "$ " + self._formatCall() + "\n" + \
				("" if not (self.showstderr and self.stderr) else ("%s\n" % self.stderr)) + \
				(("received: " + _r(self.output))
						if self.exception is None
						else str(self.exception)) + "\n" + \
			   "expected: " + _r(self.expected)

	def okay(self):
		return self.exception is None and self.compare(self.key(self.output), self.key(self.expected))

#-------------------------------------------------------------------------------


class DinnerOutputTest(Test):

	rate = 0.15

	def run(self):
		self.contents = open(in_cwd("output.txt")).read().strip()
		self.error = False
		try:
			self.fields = list(map(int, self.contents.split(";")))
		except:
			self.fields = []
			self.error = True
			return

		self.avg = sum(self.fields) / float(len(self.fields))
		self.starved_count = sum(1 for p in self.fields if (p / self.avg) < 1 - self.rate)
		
	def okay(self):
		return self.starved_count == 0 and self.fields != 0

	def what(self):
		return "output.txt: %s\n%s" % (self.contents,
				"ERROR: unexpected output format" if self.error else 
				("%d philosopher(s) starved (avg = %.1f)" % (self.starved_count, self.avg)))
		

def PhilosopherTest(num, runtime):

	return AbortingTestGroup(
			EmptyTestLine(),
			ReturnCodeTest("./dinner", [num, runtime]),
			DinnerOutputTest(),
		)

#-------------------------------------------------------------------------------

def main(cd=None):
	tests = TestGroup(
		EmptyTestLine("[INFO] No philosopher should be down by %.0f%% or more from average." % (DinnerOutputTest.rate * 100)),
		AbortingTestGroup(
			PhilosopherTest(3, 10),
			PhilosopherTest(5, 4),
		),
	)

	return tests.test()

#-------------------------------------------------------------------------------

def validate(job):
	global Test_cwd
	Test_cwd = job.working_dir

	job.run_make(mandatory=True)
	valid = main()
	output = "\n".join(msg for _, msg in output_lines)
	if valid:
		job.send_pass_result(output)
	else:
		job.send_fail_result(output)

if __name__ == "__main__":
	log = print_log
	sys.exit(0 if main() else 1)

