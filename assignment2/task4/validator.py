#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
ParProg/ST19/T2.4/v1

	$ color() (set -o pipefail; "$@" 2>&1>&3 | sed $'s,.*,\e[31m&\e[m,' >&2) 3>&1
	$ make
	$ color python3 path/to/validator.py

"""

import operator
import os
import subprocess
import sys
import urllib.request

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

class CreateFileJob(Test):

	def __init__(self, filename, contents):
		self.filename = filename
		self.contents = contents

	def run(self):
		with open(in_cwd(self.filename), "w") as f:
			f.write(self.contents)

	def okay(self):
		return True

	def what(self):
		return "Preparing %s" % self.filename
		

def ParGrepMonTest(strings_c, input_c, expected):

	return AbortingTestGroup(
			EmptyTestLine(),
			CreateFileJob("strings.txt", strings_c),
			CreateFileJob("input.txt", input_c),
			ExecutionTest("java", ["-jar", "pargrepmon.jar", "strings.txt", "input.txt"], expected, key=sorted),
		)

#-------------------------------------------------------------------------------

def main(cd=None):
	tests = TestGroup(
		AbortingTestGroup(
			EmptyTestLine("Note: Permutations of output are okay."),
			ParGrepMonTest(
				"abc\ndef\n",
				"tnu lcu irl jxu hsd duu gtj ilq fxt\n"
				"rot dar oop abc hzy def sjx tag bci\n"
				"fvl fyh jpd uyw wry nab pxr rtf ikc\n"
				"wnv rnb wwr upa def zyo tle drh wpg\n"
				"xxz xfx dfg def abc ujo oid hev def\n"
				"abc def ogz def def ngh pbf def fbh\n"
				"joz dij lvz aqx def gcx def ydo zcs\n",
				['abc;3', 'def;10']
			),
			ParGrepMonTest(
				"\n".join("the monitor concept is great".split()),
				urllib.request.urlopen("https://www.dcl.hpi.uni-potsdam.de/teaching/parProg19/assignments/hoare1974_monitors_ocrloweralnum.txt").read().decode('ascii'),
				['the;503', 'monitor;98', 'concept;9', 'is;293', 'great;4']
			)
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

