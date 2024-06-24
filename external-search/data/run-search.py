""" <data/run-search.py>
    
    A script dedicated to pipe registry data generation and sampling the running measures of the methods,
    on "pesquisa.exe".
"""

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Callable, Any


def list_apply(__funcs: list[Callable], __values: list[Any]) -> list[Any]:
    return [f(v) for f, v in zip(__funcs, __values)]


def cstructure_parse_line_values(cls: type, line: str) -> list:
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
    return cls(*cstructure_parse_line_values(cls, line))


@dataclass(init=False, repr=True, eq=True, order=False, frozen=True)
class CStructure(ABC):
    """Represents a C structure."""

    @staticmethod
    @abstractmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(CStructure, line)


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class IntTuple(CStructure):
    x: int
    y: int

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(IntTuple, line)


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
class TransparentCounter:
    reg: IntTuple
    ebst: IntTuple
    erbt: IntTuple
    b: IntTuple
    bs: IntTuple
    total_cmp: IntTuple

    @staticmethod
    def get_from_line(line: str):
        return cstructure_get_from_line(TransparentCounter, line)


@dataclass(init=True, repr=True, eq=True, order=False, frozen=True)
class PesquisaProfile:
    registry: SearchingRegistry
    transparent: TransparentCounter

    construction_time: float
    search_time: float


class PesquisaProfiler(object):
    __slots__: list[str] = [
        "__r_paths", "__a_paths"
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
            process_args, stdout=subprocess.PIPE, text=True, timeout=1, shell=False, cwd=self.__a_paths["base_dir"]
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
        elif ordering not in ["ascending", "descending", "unordered"]:
            raise ValueError

        ordering_id: int = {
            "ascending": 1,
            "descending": 2,
            "unordered": 3
        }[ordering]

        profiles: list[PesquisaProfile] = list()
        for method_id in range(4):
            try:
                profiles.append(self.__run_instance(method_id, registries_qtt, ordering_id, key))
            except RuntimeError as re:
                print(re)

        return profiles

    def generate(self, *, registries_qtt: int = 100, ordering: str = "unordered") -> None:

        ...


def test_cstructure_parsing() -> None:
    print(
        SearchingRegistry.get_from_line("1 2 nothing nothing")
    )

    print(
        TransparentCounter.get_from_line("22 0 0 0 0 0 0 0 139 211 0 421 ")
    )


if __name__ == "__main__":
    p_profiler = PesquisaProfiler()
    X = p_profiler.run_methods(registries_qtt=100, ordering="unordered", key=15)
    for x in X:
        print(x)
