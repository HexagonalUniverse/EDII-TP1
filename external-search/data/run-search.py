""" <data/run-search.py>
    
    A script dedicated to pipe registry data generation and sampling the running measures of the methods,
    on "pesquisa.exe".
"""

from dataclasses import dataclass
from typing import IO

from project_manager import ProjectManager
from cstructures import PesquisaProfile, TransparentCounter, SearchingRegistry, IntTuple


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

        return AvgSample(l, reg_qtt, method, *v)

    def write(self, __file: IO) -> None:
        """Writes a line in csv format corresponding to the dataclass information."""
        __file.write(
            ",".join(map(str,
                         [self.method, self.samples, self.cardinality, *[self.transparent[i] for i in range(12)],
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
        "__project_manager",
        "__order_dict"
    ]

    __project_manager: ProjectManager

    def __init__(self) -> None:
        self.__project_manager: ProjectManager = ProjectManager()

        self.__order_dict: dict[str, int] = {
            "ascending": 1,
            "descending": 2,
            "unordered": 3
        }

    def __fetch_profile_log(self) -> PesquisaProfile:
        with open(self.__project_manager["logging_file"], "r") as log_file:
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
        timeout: int = 200

        try:
            self.__project_manager.run_process_in_context(
                *map(str, [self.__project_manager["searching_executable"], method, qtt, situation, key]),
                timeout=timeout
            )
        except RuntimeError as re:
            raise RuntimeError("Error running instance", re)

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
                                                TransparentCounter(*(IntTuple(0, 0) for _ in range(6))),
                                                0, 0
                                                ))

        return profiles

    def generate_and_sample_avg_from_methods(self, *, registries_qtt: int = 100, file_order: str = "unordered",
                                             samples: int) -> list[AvgSample]:
        from random import randint

        self.__project_manager.generate_input(registries_qtt=registries_qtt, file_order=file_order)

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
                AvgSample.get(i, registries_qtt, [methods_data[j][i] for j in range(samples)])
            )
        return avg_samples

    def generate_and_sample_avg_b_bp(self, *, registries_qtt: int = 100, samples: int = 5) \
            -> tuple[AvgSample, AvgSample]:
        from random import randint

        self.__project_manager.generate_input(registries_qtt=registries_qtt, file_order="unordered")

        b_data: list[PesquisaProfile] = []
        bp_data: list[PesquisaProfile] = []
        for _ in range(samples):
            key: int = randint(0, registries_qtt - 1)
            b_data.append(self.__run_instance(method=2, qtt=registries_qtt, situation=3, key=key))
            bp_data.append(self.__run_instance(method=3, qtt=registries_qtt, situation=3, key=key))

        return AvgSample.get(2, registries_qtt, b_data), AvgSample.get(3, registries_qtt, bp_data)

    def sample_avg_ebst(self, *, registries_qtt: int = 100, samples: int = 5) -> AvgSample:
        from random import randint

        ebst_data: list[PesquisaProfile] = []

        for _ in range(samples):
            key: int = randint(0, registries_qtt - 1)
            ebst_data.append(self.__run_instance(method=1, qtt=registries_qtt, situation=1, key=key))
        return AvgSample.get(1, registries_qtt, ebst_data)


def sample_average_same_key_log_scale(file_order: str = "ascending", limit: int = 1_000, base: int = 10,
                                      samples: int = 5, filename: str = None) -> None:
    """Samples and stores the data from running an average of #samples of <pesquisa.exe> profiling, with the registries
    data file ranging in number logarithmically from (base) to (base * floor{log_{base}{limit}}) and the file_order,
    for each searching method, into the passed filename."""

    import numpy as np
    from time import time
    r: int = int(np.round(np.log(limit) / np.log(base)))
    limit: int = base ** r

    if filename is None:
        filename = f"samples/all-avg{samples}-{file_order}-sk-log-{limit}.csv"

    file: IO = open(filename, "a")
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


def sample_average_lin_b_bp_tree(start: int = 100, gap: int = 100, bound: int = 10_000, samples: int = 5,
                                 filename: str = None) -> None:
    from time import time

    if filename is None:
        filename = f"samples/bb+-avg{samples}-lin-{bound}.csv"

    file: IO = open(filename, "a")
    AvgSample.write_file_header(file)

    profiler = PesquisaProfiler()

    # Measuring the sampling time...
    last_time: float
    current_time: float
    last_time_measure: float | None
    current_time_measure: float | None = None

    for card in range(start, bound + 1, gap):
        print(f"{filename}: #{card}. ", end='')

        last_time = time()
        avg_samples: tuple[AvgSample, AvgSample] = profiler.generate_and_sample_avg_b_bp(
            registries_qtt=card,samples=samples
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


def sample_average_lin_ebst_erbt(start: int = 100, gap: int = 100, bound: int = 1_000, samples: int = 5,
                                 filename: str = None) -> None:
    from time import time

    if filename is None:
        filename = f"samples/ebst_erbt+-avg{samples}-lin-{bound}.csv"

    file: IO = open(filename, "r+")
    AvgSample.write_file_header(file)

    profiler = PesquisaProfiler()
    p_manager: ProjectManager = ProjectManager()

    # Measuring the sampling time...
    last_time: float
    current_time: float
    last_time_measure: float | None
    current_time_measure: float | None = None

    p_manager.rebuild("-D IMPL_ERBT_ONLY=false", debug_mode=None, transparent_mode=True)
    print("EBST ---\n")
    file.write("ebst\n")

    for card in range(start, bound + 1, gap):
        print(f"{filename}: #{card}. ", end='')
        break
        p_manager.generate_input(registries_qtt=card, file_order="ascending")

        last_time = time()
        avg_sample: AvgSample = profiler.sample_avg_ebst(registries_qtt=card, samples=samples)
        current_time = time()

        last_time_measure = current_time_measure
        current_time_measure = current_time - last_time

        if last_time_measure is None:
            last_time_measure = current_time_measure

        time_incr_per: float = round(100 * (current_time_measure - last_time_measure) / last_time_measure, 2)
        print(f"Sampling took {current_time_measure} [s] "
              f"({'+' if time_incr_per > 0 else ''}{time_incr_per}%).")

        avg_sample.write(file)

    p_manager.rebuild("-D IMPL_ERBT_ONLY=true", debug_mode=None, transparent_mode=True)
    print("ERBT ---\n")
    file.write("erbt\n")
    # for card in range(start, bound + 1, gap):
    for card in range(35000, bound + 1, 5_000):
        print(f"{filename}: #{card}. ", end='')
        p_manager.generate_input(registries_qtt=card, file_order="ascending")

        last_time = time()
        avg_sample: AvgSample = profiler.sample_avg_ebst(registries_qtt=card, samples=samples)
        current_time = time()

        last_time_measure = current_time_measure
        current_time_measure = current_time - last_time

        if last_time_measure is None:
            last_time_measure = current_time_measure

        time_incr_per: float = round(100 * (current_time_measure - last_time_measure) / last_time_measure, 2)
        print(f"Sampling took {current_time_measure} [s] "
              f"({'+' if time_incr_per > 0 else ''}{time_incr_per}%).")

        avg_sample.write(file)
    file.close()


def test_cstructure_parsing() -> None:
    print(
        SearchingRegistry.get_from_line("1 2 nothing nothing")
    )

    print(
        TransparentCounter.get_from_line("22 0 0 0 0 0 0 0 139 211 0 421 ")
    )


def __official_sampling_1() -> None:
    sample_average_same_key_log_scale(
        file_order="ascending", limit=1_000_000, base=10, samples=5
    )


def __official_sampling_2() -> None:
    sample_average_lin_ebst_erbt(
        start=1_000, gap=1_000, bound=100_000, samples=5
    )


def __official_sampling_3() -> None:
    sample_average_lin_b_bp_tree(
        start=1_000, gap=1_000, bound=100_000, samples=5
    )


if __name__ == "__main__":
    # __official_sampling_1()
    __official_sampling_2()
    # __official_sampling_3()