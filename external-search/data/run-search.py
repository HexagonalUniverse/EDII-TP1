""" <data/run-search.py>
    
    A script dedicated to pipe registry data generation and sampling the running measures of the methods,
    on "pesquisa.exe".
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Callable, Any, IO


def list_apply(__funcs: list[Callable], __values: list[Any]) -> list[Any]:
    return [f(v) for f, v in zip(__funcs, __values)]


def cstructure_parse_line_values(cls: type, line: str) -> list[Any]:
    tokens: list[str] = line.split(" ")
    values: list = list()
    for field_type in cls.__annotations__.values():
        if hasattr(field_type, "__annotations__"):
            l: int = len(field_type.__annotations__)
            values.append(
                field_type(*tokens[:l])
            )
            tokens = tokens[l:]
            continue
        values.append(
            field_type(tokens[0])
        )
        tokens = tokens[1:]
    return values


def cstructure_get_from_line(cls: type, line: str):
    return cls(* cstructure_parse_line_values(cls, line))


@dataclass(init=False, repr=True, eq=True, order=False, frozen=True)
class CStructure(ABC):
    """Represents a C structure."""

    @staticmethod
    @abstractmethod
    def get_from_line(line: str): ...

    def line_repr(self) -> str:
        return " ".join(str(self.__getattribute__(a)) for a in self.__annotations__.keys())


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class IntTuple(CStructure):
    x: int
    y: int

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(IntTuple, line)

    def __getitem__(self, item: int) -> int:
        if item == 0:
            return self.x
        elif item == 1:
            return self.y
        raise IndexError


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class SearchingRegistry(CStructure):
    key: int
    data_1: int
    data_2: str
    data_3: str

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(SearchingRegistry, line)


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class TransparentCounter(CStructure):
    reg: IntTuple
    ebst: IntTuple
    erbt: IntTuple
    b: IntTuple
    bs: IntTuple
    total_cmp: IntTuple

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(TransparentCounter, line)

    def __getitem__(self, index: int) -> int:
        if index >= 12 or index < 0:
            raise IndexError
        d: int = index // 2
        r: int = index % 2

        match d:
            case 0:
                return self.reg[r]
            case 1:
                return self.ebst[r]
            case 2:
                return self.ebst[r]
            case 3:
                return self.b[r]
            case 4:
                return self.bs[r]
            case 5:
                return self.total_cmp[r]
        raise ValueError


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class PesquisaProfile(CStructure):
    registry: SearchingRegistry
    transparent: TransparentCounter

    construction_time: float
    search_time: float

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(PesquisaProfile, line)


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class AvgSample(object):
    samples: int
    cardinality: int

    method: int
    transparent: list[float]

    construction_time: float
    search_time: float

    @staticmethod
    def get(method: int, reg_qtt: int, __profiles: list[PesquisaProfile]):
        v: list = [[0 for _ in range(12)], 0, 0]
        l: int = 0

        for i in range(12):
            v[0][i] = int(__profiles[0].transparent[i])

        for p in __profiles:
            v[1] += p.construction_time
            v[2] += p.search_time
            l += 1

        for j in range(1, 3):
            v[j] /= l

        return AvgSample(l, reg_qtt, method, * v)

    def write(self, __file: IO) -> None:
        """Writes a line in csv format corresponding to the dataclass information."""
        __file.write(
            ",".join(map(str,
                         [self.method, self.samples, self.cardinality, * [self.transparent[i] for i in range(12)],
                          self.construction_time, self.search_time]
                         )) + "\n"
        )

    @staticmethod
    def write_file_header(__file: IO) -> None:
        __file.write(
            " ".join(["method", "samples", "card", "transparent[12]", "c_time", "s_time"]) + "\n"
        )


class PesquisaProfiler(object):
    __slots__: list[str] = [
        "__r_paths", "__a_paths",
        "__order_dict"
    ]

    __r_paths: dict[str, str]
    __a_paths: dict[str, str]

    def __init__(self) -> None:
        from os import path

        # Base project directory. <external-search/>
        base_dir: str = path.abspath(path.join(__file__, "..\\.."))

        # Relative filepaths from <external-search/>.
        self.__r_paths: dict[str, str] = {
            "dir_cache": "temp/cache/",
            "dir_sample": "data/samples/",

            "logging_file": "temp/cache/_last_pesquisa_run.log",
            "input_datafile": "temp/input-data.bin",
            "searching_executable": "bin/exe/pesquisa.exe",
            "data_gen_executable": "bin/exe/data-gen.exe",
        }

        # Absolute filepaths. Mapping path.abspath over __r_paths.
        self.__a_paths: dict[str, str] = dict()
        for key in self.__r_paths.keys():
            self.__a_paths[key] = path.abspath(path.join(base_dir, self.__r_paths[key]))
        self.__a_paths["base_dir"] = base_dir

        self.__order_dict: dict[str, int] = {
            "ascending": 1,
            "descending": 2,
            "unordered": 3
        }

    def __fetch_profile_log(self) -> PesquisaProfile:
        with open(self.__a_paths["logging_file"], "r") as log_file:
            reg_line: str = log_file.readline()
            time_measures: list[float] = list(map(float, log_file.readline().split(" ")))
            transparent_counter_line: str = log_file.readline()

        if len(time_measures) != 2:
            raise ValueError

        return PesquisaProfile(SearchingRegistry.get_from_line(reg_line),
                               TransparentCounter.get_from_line(transparent_counter_line),
                               *time_measures)

    def __run_instance(self, method: int, qtt: int, situation: int, key: int) -> PesquisaProfile:
        """Runs a single instance (search handle session) of "pesquisa.exe", given the parameters.
        Unprotected in parameters."""
        import subprocess

        process_args: list[str] = list(map(str, [self.__a_paths["searching_executable"], method, qtt, situation, key]))

        process_result: subprocess.CompletedProcess
        process_result = subprocess.run(
            process_args, stdout=subprocess.PIPE, text=True, timeout=10, shell=False, cwd=self.__a_paths["base_dir"]
        )

        if process_result.returncode != 0:
            raise RuntimeError(f"Exited with code {process_result.returncode}")

        return self.__fetch_profile_log()

    def run_methods(self, *, registries_qtt: int = 100, ordering: str = "unordered",
                    key: int = 0) -> list[PesquisaProfile]:
        """Runs and returns the profile of all searching handles for a given data file and key."""

        if not isinstance(registries_qtt, int):
            raise TypeError
        elif registries_qtt <= 0:
            raise ValueError

        if not isinstance(ordering, str):
            raise TypeError
        elif ordering not in self.__order_dict.keys():
            raise ValueError

        ordering_id: int = self.__order_dict[ordering]

        profiles: list[PesquisaProfile] = list()
        for method_id in range(4):
            try:
                profiles.append(self.__run_instance(method_id, registries_qtt, ordering_id, key))
            except RuntimeError:
                profiles.append(PesquisaProfile(SearchingRegistry(0, 0, "undef", "undef"),
                                                TransparentCounter(* (IntTuple(0, 0) for _ in range(6))),
                                                0, 0
                                                ))

        return profiles

    def generate(self, *, registries_qtt: int = 100, file_order: str = "unordered") -> None:
        import subprocess

        if not isinstance(registries_qtt, int):
            raise TypeError
        elif registries_qtt <= 0:
            raise ValueError

        if not isinstance(file_order, str):
            raise TypeError
        elif file_order not in ["ascending", "ascending-gap", "descending",
                                "descending-gap", "unordered", "unordered-gap"]:
            raise ValueError

        file_order_id: int = ["ascending", "ascending-gap", "descending",
                              "descending-gap", "unordered", "unordered-gap"].index(file_order)

        process_result: subprocess.CompletedProcess
        process_result = subprocess.run(
            [self.__a_paths["data_gen_executable"], str(file_order_id), str(registries_qtt),
             self.__a_paths["input_datafile"]],
            stdout=subprocess.PIPE, text=True, timeout=120, shell=False, cwd=self.__a_paths["base_dir"]
        )

        if process_result.returncode != 0:
            raise RuntimeError

    def generate_and_sample_avg_from_methods(self, *, registries_qtt: int = 100, file_order: str = "unordered",
                                             samples: int) -> list[AvgSample]:
        from random import randint

        self.generate(registries_qtt=registries_qtt, file_order=file_order)

        # Sampling
        methods_data: list[list[PesquisaProfile]] = list()
        for _ in range(samples):
            methods_data.append(
                self.run_methods(registries_qtt=registries_qtt, ordering=file_order,
                                 key=randint(0, registries_qtt - 1))
            )

        avg_samples: list[AvgSample] = list()
        for i in range(4):
            avg_samples.append(
                AvgSample.get(i,registries_qtt, [methods_data[j][i] for j in range(samples)])
            )
        return avg_samples


def sample_average_same_key_log_scale(file_order: str = "ascending", limit: int = 1_000, base: int = 10,
                                      samples: int = 5, filename: str = None) -> None:
    """Samples and stores the data from running an average of #samples of <pesquisa.exe> profiling, with the registries
    data file ranging in number logarithmically from (base) to (base * floor{log_{base}{limit}} and the file_order,
    for each searching method, into the passed filename."""

    import numpy as np
    from time import time
    r: int = int(np.round(np.log(limit) / np.log(base)))
    limit: int = base ** r

    if filename is None:
        filename = f"samples/all-avg{samples}-{file_order}-sk-log-{limit}.csv"

    file: IO = open(filename, "w")
    AvgSample.write_file_header(file)

    profiler = PesquisaProfiler()

    # Measuring the sampling time...
    last_time: float
    current_time: float
    last_time_measure: float | None
    current_time_measure: float | None = None

    for card in [base ** i for i in range(1, r + 1)]:
        print(f"{filename}: #{card}. ", end='')

        last_time = time()
        avg_samples: list[AvgSample] = profiler.generate_and_sample_avg_from_methods(
            registries_qtt=card, file_order=file_order, samples=samples
        )
        current_time = time()

        last_time_measure = current_time_measure
        current_time_measure = current_time - last_time

        if last_time_measure is None:
            last_time_measure = current_time_measure

        time_incr_per: float = round(100 * (current_time_measure - last_time_measure) / last_time_measure, 2)
        print(f"Sampling took {current_time_measure} [s] "
              f"({'+' if time_incr_per > 0 else ''}{time_incr_per}%).")

        for y in avg_samples:
            y.write(file)

    file.close()


def test_cstructure_parsing() -> None:
    print(
        SearchingRegistry.get_from_line("1 2 nothing nothing")
    )

    print(
        TransparentCounter.get_from_line("22 0 0 0 0 0 0 0 139 211 0 421 ")
    )


if __name__ == "__main__":
    p_profiler = PesquisaProfiler()
    p_profiler.generate(registries_qtt=100_000, file_order="ascending")

    exit(1)
    sample_average_same_key_log_scale(
        file_order="ascending", limit=10_000, base=10, samples=5
    )

