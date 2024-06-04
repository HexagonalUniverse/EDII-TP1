""" <o_6t.py>: One scrip-file version of Hexadic tester.

    A script to handle tests.
    =========================

    The tests should be lied structured in a unique test file. In this test file, the program should be specified,
    as well as the expected input and outputs. Check the details of the test data syntax at README.md.

    To run test cases, call the script at a terminal environment as
    "python <path>6t.py test-1.6t test-2.6t ... test-n.6t"
    and they will be all first be sequentially attempted parsed and, after, run.

    The program instantly exits in case of a parsing error in one of the test cases, with a negative return code. In
    case of fail of one test, the other tests will be continued being tested despite unique failures. The process
    terminates with exit code different of 0, equal to the number of cases that had failed.

    On comparing the output of a program execution for a case with the expected one, ANSI escape code sequences
    from the output are neglected, and both are stripped on both ends.
"""

from typing import TypeAlias, Iterator, Any, Callable
from dataclasses import dataclass
import subprocess


""" <parser.py>: Where the parser for the tests is written. """


class ParserError(Exception):
    """Represents an error faced during parsing."""

    def __init__(self, error_message: str) -> None:
        super().__init__(error_message)


class UnmatchInputOutput(ParserError):
    ...


# The sequences, data types there is.
SequenceType: TypeAlias = str | list[str]


@dataclass(init=True, repr=True, eq=True, order=False, frozen=False)
class TestData(object):
    """An object representing the test data - of test cases - to be run."""

    program: str | None = None
    input: list[SequenceType] | None = None
    expected_output: list[SequenceType] | None = None

    def __bool__(self) -> bool:
        return self.program is not None and self.input is not None and self.expected_output is not None


def seek_leftmost_control_seq(__line: str) -> str | None:
    """Attempts identified what is the leftmost control sequence mark in the (left-stripped) line.
    Returns an empty string in case none is identified. """

    for mark in ["#", ":::"]:
        if __line.startswith(mark):
            return mark
    return ""


def strip_line(__line: str) -> str:
    return __line.lstrip(" \n").rstrip(" \n")


def parse_sequence(line: str, stream_iterator: Iterator[str], end_control_seq,
                   obj: Any, object_combinator: Callable[[str, Any], None]) -> Any:
    # Verifying if the sequence is inline.
    ecs_pos: int = line.find(end_control_seq)
    if ecs_pos >= 0:
        # Respectively, the parts before and after the end control seq.
        line_seq_part = line[:ecs_pos]
        line_leftover = line[ecs_pos + len(end_control_seq):]

        # If the leftover part equals the stripped version of it, then
        # it means there is not proper spacing between the marker in the text.
        if line_leftover and line_leftover == strip_line(line_leftover):
            raise ParserError("There's no proper spacing between the data control sequence and the text.")

        object_combinator(line_seq_part, obj)
        return obj

    # Retrieving what is left from the line.
    object_combinator(line, obj)

    # Retrieving data until end control sequence is found.
    for line in stream_iterator:
        ecs_pos: int = line.find(end_control_seq)
        # In case reached the end control sequence.
        if ecs_pos >= 0:
            object_combinator(line[:ecs_pos], obj)
            return obj

        # Otherwise continues agglomerating the characters buffer.
        object_combinator(line, obj)
    raise ParserError("Sequence never finished.")


def parse_list(line_leftover: str, stream_iterator: Iterator) -> list[str]:
    def assemble(line: str, __obj: list[str]) -> None:
        stripped_line: str = strip_line(line)
        if stripped_line:
            __obj.append(strip_line(line))

    obj: list[str] | None = list()
    return parse_sequence(line_leftover, stream_iterator, ":::", obj, assemble)


def parse_text(line_leftover: str, stream_iterator: Iterator) -> str:
    def assemble(line: str, __obj: list[str]) -> None:
        __obj[0] += line

    # Puts the string into a list so the pointer reference to it makes so that alterations in assemble
    # are not lost.
    obj: list[str] = [""]
    return parse_sequence(line_leftover, stream_iterator, "#", obj, assemble)[0]


control_sequences_parsers: dict[str, Callable[[str, Iterator], SequenceType | None]] = {
    ":::": parse_list,
    "#": parse_text,
}


def parse_data_block(line_leftover: str, stream_iterator: Iterator) -> SequenceType:
    # Continues seeking from the stripped part of it.
    line_leftover: str = line_leftover.lstrip(" \n")

    # If there is something left on the line, then it must start with the control sequence.
    if line_leftover:
        control_seq: str = seek_leftmost_control_seq(line_leftover)
        if not control_seq:
            raise ParserError("Expected data control sequence, but found obstacle inline.")

    else:
        # Otherwise, seeks for the next occurrence of control sequence in another line.

        control_seq: str | None = None

        for line in stream_iterator:
            stripped_line: str = strip_line(line)

            # In case it being an empty line, ignores.
            if not stripped_line:
                continue

            control_seq: str = seek_leftmost_control_seq(stripped_line)
            if not control_seq:
                raise ParserError("Expected data control sequence, but found obstacle in forward line.")
            line_leftover = stripped_line
            break

    # At this point, control_seq is already defined; so it can be consumed from the current analyzed line.
    line_leftover = line_leftover[len(control_seq):]

    # Checking proper spacing between the marker in the text.
    if line_leftover and line_leftover == strip_line(line_leftover):
        raise ParserError("There's no proper spacing between the data control sequence and the text.")

    # Hashing the right parser for the control sequence.
    sequence_parser: Callable[[str, Iterator], SequenceType] = control_sequences_parsers[control_seq]

    return sequence_parser(line_leftover, stream_iterator)


class TestDataParser(object):
    """A simple parser object for a test file."""

    __slots__: list[str] = [
        # Parameters
        "verbose",

        # File stream
        "__test_filename", "__test_file",

        # Tracing information
        "current_line", "return_code",
        "__testdata",
        "__input_delimiters", "__output_delimiters"
    ]

    verbose: bool

    __test_filename: str
    __test_file: Iterator

    current_line: int
    __testdata: TestData
    __input_delimiters: list[[int, int]]
    __output_delimiters: list[[int, int]]

    def __init__(self, verbose: bool = False) -> None:
        self.verbose = verbose

        self.current_line: int = 0

        self.__input_delimiters: list[[int, int]] = []
        self.__output_delimiters: list[[int, int]] = []

    def __repr__(self) -> str:
        return f"Parser\n\tinputs: {repr(self.__input_delimiters)}\n\toutputs: {repr(self.__output_delimiters)}"

    """The parser is an iterator over the file-stream at which the parser is being done."""

    def __iter__(self):
        return self

    def __next__(self):
        self.current_line += 1
        return next(self.__test_file)

    def __parse_line(self, line: str) -> bool:
        """Parses a single line of the stream. """
        from os import path

        line: str = strip_line(line)

        if line.startswith("PROGRAM:"):
            if not self.__testdata.program:
                self.__testdata.program = path.join(
                    path.join(self.__test_filename, "../"), line[len("PROGRAM:") + 1:]
                )
            else:
                self.return_code = -1
                raise ParserError("Multiple program directives definitions on test file.\n")

        elif line.startswith("IN:"):
            self.__input_delimiters.append([self.current_line, -1])

            self.__testdata.input.append(
                parse_data_block(line[len("IN:") + 1:], self)
            )

            self.__input_delimiters[-1][1]: int = self.current_line

        elif line.startswith("OUT:"):
            self.__output_delimiters.append([self.current_line, -1])

            self.__testdata.expected_output.append(
                parse_data_block(line[len("OUT:") + 1:], self)
            )

            self.__output_delimiters[-1][1]: int = self.current_line

        return True

    def __parse(self) -> TestData:
        """Where the actual parsing takes act at."""

        with open(self.__test_filename, "r") as self.__test_file:
            self.current_line: int = 0

            for line in self:
                if not self.__parse_line(strip_line(line)):
                    break

        # Errors*
        if len(self.__testdata.input) != len(self.__testdata.expected_output):
            raise UnmatchInputOutput(f"The number of inputs ({len(self.__testdata.input)}) doesn't match the number "
                                     f"of outputs ({len(self.__testdata.expected_output)}).")

        return self.__testdata

    def parse(self, test_filename: str) -> TestData:
        """Parses the test data file with the given filename. Raises ParserError in case of one was detected
                during the parsing."""

        self.__testdata: TestData = TestData()
        self.__test_filename = test_filename

        # Respectively, where each input and output starts and finishes (in lines).
        self.__input_delimiters.clear()
        self.__output_delimiters.clear()

        self.__testdata.input = list()
        self.__testdata.expected_output = list()

        try:
            return self.__parse()

        except UnmatchInputOutput as p_err:
            if len(self.__testdata.input) > len(self.__testdata.expected_output):
                begin, end = self.__input_delimiters[-1]
                error_msg: str = f"The input from lines {begin}-{end} misses a counterpart output."
                raise UnmatchInputOutput(error_msg) from p_err
            else:
                begin, end = self.__output_delimiters[-1]
                error_msg: str = f"The output from lines {begin}-{end} misses a counterpart input."
                raise UnmatchInputOutput(error_msg) from p_err


""" <runner.py>: Where the runner for the tests is written. """


@dataclass(init=True, repr=True, eq=True, order=False, frozen=False)
class TestCase(object):

    program: str | None = None
    input: list[SequenceType] | None = None
    expected_output: list[SequenceType] | None = None


def remove_aec(printed_sequence: str) -> str:
    """Remove the ANSI escape code sequences from an output print string."""

    _ESCAPE: str = "\x1b"   # ANSI escape code introducer.
    _EXIT: str = "m"        # ANSI escape code termination signaler.

    while True:
        # Continuously seeks and removes the first escape sequence from the
        # string until termination.
        x: int = printed_sequence.find(_ESCAPE)
        if x < 0:
            break
        post_part: str = printed_sequence[x + len(_ESCAPE):]
        printed_sequence = post_part[post_part.find(_EXIT) + len(_EXIT):]
    return printed_sequence


def format_output(output: str) -> str:
    """Returns the output formatted, for proper comparison."""
    # Removing possible escape, not printable sequences from the output.
    output = remove_aec(output)

    # Stripping the answer.
    output = strip_line(output)

    return output


def compare_output_test(expected_output: SequenceType, output: str) -> bool:
    """Compares the output of a run test with the expected one."""

    if isinstance(expected_output, str):
        # Removing possible escape, not printable sequences from the output.
        output = remove_aec(output)

        # Stripping both answers.
        output = strip_line(output)
        expected_output = strip_line(expected_output)

        # print(f"<{repr(expected_output)}> == <{repr(output)}>:", expected_output == output)

        # Finally, computing the comparison.
        return expected_output == output
    raise NotImplementedError


def run_single_test_case(__program: str, __input: SequenceType, __expected_output: SequenceType) -> str:
    """Attempts running a single test-case from the test-data, returning the output it raised."""

    input_seq: SequenceType = __input
    if input_seq is None:
        input_seq = []

    process_result: subprocess.CompletedProcess
    if isinstance(input_seq, list):
        # In case the input-sequence is a list, passes it as arguments to the application.
        input_seq: list[str]
        process_result = subprocess.run(
            [__program, *input_seq], stdout=subprocess.PIPE,
            text=True, timeout=1, shell=False
        )
    elif isinstance(input_seq, str):
        # Otherwise, if it is a string, feed in by regular input.
        input_seq: str
        process_result = subprocess.run(
            [__program], input=input_seq,
            stdout=subprocess.PIPE,
            text=True, timeout=1, shell=False,
        )
    else:
        raise TypeError(type(input_seq))

    if process_result.returncode != 0:
        raise RuntimeError(f"Program failed with return code {process_result.returncode}.")
    return process_result.stdout


class TestDataRunner(object):

    __slots__: list[str] = [
        "__test_failures"
    ]

    def __init__(self):
        self.__test_failures: list[tuple[int, SequenceType, SequenceType, SequenceType]] = list()

    def __run_test(self, __testdata: TestData) -> int:
        self.__test_failures.clear()

        failure_count: int = 0
        for input_seq, expected_output, test_case_id \
                in zip(__testdata.input, __testdata.expected_output, range(1, len(__testdata.input) + 1)):

            output: str = run_single_test_case(__testdata.program, input_seq, expected_output)
            if not compare_output_test(expected_output, output):
                failure_count += 1

                self.__test_failures.append(
                    (test_case_id, input_seq, expected_output, output)
                )

        return failure_count

    def run_test(self, __testdata: TestData) -> int:
        """Attempts running the given test-data, returning the number of failures."""
        if not isinstance(__testdata, TestData):
            raise TypeError

        # It seems that, when tempting executing the program in linux,
        # the relative path is not recognized properly...
        from os import path
        __testdata.program = path.abspath(__testdata.program)

        # print(f"Running {__testdata}")
        return self.__run_test(__testdata)

    def print_case_errors(self) -> None:
        if not self.__test_failures:
            return None

        print("\x1b[0;35m#id:\t<expected> != <out>\x1b[0;37m:\n")
        for case in self.__test_failures:
            case_id, input_seq, expected_out, out = case

            expected_out = format_output(expected_out)
            out = format_output(out)

            print(f"\x1b[0;31m#{case_id}\x1b[0;37m:\t"
                  f"\x1b[0;34m{repr(expected_out)}\x1b[0;37m != "
                  f"\x1b[0;31m{repr(out)}\x1b[0;37m\n")

        return None


""" Interface
    ========= """


def check_filename_extension(filename: str) -> bool:
    for ext in [".txt", ".6t"]:
        if filename.endswith(ext):
            return True
    return False


def validate_file(filepath: str) -> bool:
    from os import path

    if not path.exists(filepath):
        print(f"Specified file <{arg}> does not exists.")
        return False
    if not check_filename_extension(filepath):
        print(f"Specified file <{arg}> does not have the right extension.")
        return False
    return True


if __name__ == "__main__":
    import sys

    args: list[str]
    _, *args = sys.argv

    Parser = TestDataParser()
    Runner = TestDataRunner()

    parsed_data: list[tuple[str, TestData]] = list()

    for arg in args:
        if not validate_file(arg):
            exit(-1)

        test_data: TestData = Parser.parse(arg)
        parsed_data.append((arg, test_data))

    exit_code: int = 0
    for arg, test_data in parsed_data:
        test_data: TestData

        failures_count = Runner.run_test(test_data)
        print(f"<{arg}>:", end=' ')
        if failures_count > 0:
            exit_code += 1
            print("\x1b[0;31mFailed", end='')
        else:
            print("\x1b[0;32mPassed ", end='')
        print(f"\x1b[0;37m (Matched {len(test_data.input) - failures_count} of {len(test_data.input)})")

        if failures_count > 0:
            Runner.print_case_errors()
    exit(exit_code)
