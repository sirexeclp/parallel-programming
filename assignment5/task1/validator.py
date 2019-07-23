#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

"""
ParProg/ST19/T5.1/v1

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
		

class HeatmapOutputTest(Test):

	def __init__(self, expected):
		self.expected = expected

	def run(self):
		self.exists = True
		try:
			self.contents = open(in_cwd("output.txt")).read().strip()
		except FileNotFoundError:
			self.exists = False
		
	def okay(self):
		return self.contents == self.expected

	def what(self):
		if not self.exists:
			return "output.txt not found"
		return "contents of output.txt:\n%s\n%s" % (self.contents,
				("" if self.okay() else ("expected:\n%s\n" % self.expected)))


def HeatmapTest(width, height, rounds, hotspotfile_c, expected, coordinates_c=None):

	args = [width, height, rounds, "hotspots.csv"]
	mpirun_flags = "--oversubscribe -np 16".split()
	if coordinates_c is not None:
		args.append("coords.csv")

	return AbortingTestGroup(
			CreateFileJob("hotspots.csv", hotspotfile_c),
			CreateFileJob("coords.csv", coordinates_c) if coordinates_c is not None else EmptyTestLine(),
			ReturnCodeTest("mpirun", mpirun_flags + ["./heatmap"] + args, showstderr=True),
			HeatmapOutputTest(expected),
		)

#-------------------------------------------------------------------------------

def main(cd=None):
	tests = TestGroup(
		AbortingTestGroup(
			HeatmapTest(20, 7, 17,
			            "x,y,startround,endround\n"
						"5,2,0,20\n"
						"15,5,5,15",
				        "11112221111111111100\n"
				        "11123432111111111110\n"
				        "11124X42211111111111\n"
				        "11124442111111222111\n"
				        "11122222111112222211\n"
				        "11111211111112232211\n"
				        "01111111111111222111"),
			HeatmapTest(20, 7, 5,
			            "x,y,startround,endround\n"
						"5,2,0,20\n"
						"15,5,5,15",
						"00111111100000000000\n"
						"00113331100000000000\n"
						"00113X31100000000000\n"
						"00113331100000000000\n"
						"00111111100000000000\n"
						"000111110000000X0000\n"
						"00000000000000000000"),
			HeatmapTest(50, 20, 32,
			            "x,y,startround,endround\n"
						"8,4,0,16\n"
						"15,10,0,40\n"
						"26,10,5,40\n"
						"40,15,20,30",
						"00011111111111110000000000000000000000000000000000\n"
						"01111111111111111110000000000000000000000000000000\n"
						"01111111111111111111111111111000000000000000000000\n"
						"01111111111111111111111111111110000000000000000000\n"
						"01111111221111111111111111111111000000000000000000\n"
						"01111111222221111111111111111111100000000000000000\n"
						"01111111222222222111111111111111110000000000000000\n"
						"01111111122222222221111122222111110000000000000000\n"
						"01111111112223333222111223332211111000000000000000\n"
						"01111111112223454322112234443211111000000000000000\n"
						"001111111112235X5322212234X43211111001111110000000\n"
						"00011111111223454322112234443211111111111111100000\n"
						"00001111111122333222111223332211111111111111100000\n"
						"00000111111122222221111122222111111111122211110000\n"
						"00000011111111222111111111111111111111223221110000\n"
						"00000001111111111111111111111111111111233321110000\n"
						"00000000111111111111111111111111001111223221110000\n"
						"00000000001111111111111111111110000111122211110000\n"
						"00000000000011111111100111110000000011111111100000\n"
						"00000000000000000000000000000000000001111111000000")
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

