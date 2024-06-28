""" <plot.py>

    Graphs the project results. Bases on matplotlib."""


import matplotlib
from matplotlib import pyplot as plt
from abc import abstractmethod
from project_manager import ProjectManager
from typing import IO


class ProjectPlotter(object):
    def __init__(self) -> None:
        self.__project_manager: ProjectManager = ProjectManager()

    def plot_avg_methods(self, r_filename: str) -> None:
        """(...)"""

        # Parsing the data
        # ----------------

        method: int
        samples: int
        card: int
        transparent: list[int]
        c_time: float
        s_time: float

        parsed_data: list[list[int | float]] = []

        data_file: IO = open(r_filename, "r")
        data_file.readline()
        for line in data_file:
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            parsed_data.append([method, samples, card, transparent, c_time, s_time])
            print(parsed_data[-1])
        data_file.close()

        data: dict[str, list[int | float]] = {
            "ISS": list(filter(lambda __list: __list[0] == 0, parsed_data)),
            "EBST": list(filter(lambda __list: __list[0] == 1, parsed_data)),
            "B": list(filter(lambda __list: __list[0] == 2, parsed_data)),
            "B+": list(filter(lambda __list: __list[0] == 3, parsed_data)),
        }

        print("b values:")
        b_values: list[int | float] = list(filter(lambda __list: __list[0] == 2, parsed_data))
        print(b_values)

        c_time_values: list[float] = [__list[-2] for __list in b_values]
        s_time_values: list[float] = [__list[-1] for __list in b_values]
        entries_dimension: list[int] = [__list[2] for __list in b_values]
        print(c_time_values)
        print(s_time_values)
        print(entries_dimension)

        # Plotting the values
        # -------------------

        fig = plt.figure(figsize=(20, 10))
        spec = fig.add_gridspec(2, 3)
        axes: list = [fig.add_subplot(spec[i // 3, i % 3]) for i in range(5)]

        for key in data:
            for __list in data[key]:
                print(f"{key}: {__list}")

        # axes[0]: Construction Time.
        for key in data:
            axes[0].plot(entries_dimension, [1000 * __list[-2] for __list in data[key]], alpha=0.5, label=key)

        # axes[1]: Searching Time.
        for key in data:
            axes[1].plot(entries_dimension, [1000 * __list[-1] for __list in data[key]], alpha=0.5, label=key)

        for key in data:
            axes[2].plot(entries_dimension, [__list[3][-2] for __list in data[key]], alpha=0.5, label=key)

        # axes[2]
        for key in data:
            axes[3].plot(entries_dimension, [__list[3][-1] for __list in data[key]], alpha=0.5, label=key)

        # axes[3]
        for key in data:
            axes[4].plot(entries_dimension, [sum(__list[3][:-2]) for __list in data[key]], alpha=0.5, label=key)

        # Stylizing
        # ---------

        for axis in axes:
            axis.grid()
            axis.legend(list(data))
            axis.set_xlabel("# Registries [u.]")
            axis.set_xscale("log")

        fig.suptitle("Avg. (5) log. Searching Data Structures")

        axes[0].set_yscale("log")
        axes[0].set_title("(log x log) Construction Time")
        axes[0].set_ylabel("Construction Time [ms]")

        axes[1].set_title("(log x lin) Searching Time")
        axes[1].set_ylabel("Searching Time [ms]")

        axes[2].set_yscale("log")
        axes[2].set_title("(log x log) Comparisons Counting in Build")
        axes[2].set_ylabel("Comparison Count in Building")

        axes[3].set_yscale("log")
        axes[3].set_title("(log x log) Comparisons Counting in Search")
        axes[3].set_ylabel("Comparison Count in Searching")

        axes[4].set_yscale("log")
        axes[4].set_title("(log x lin) Total IO Page Transference")
        axes[4].set_ylabel("Total Page Transference")

        # Displaying it
        plt.show()
        return None


if __name__ == "__main__":
    p_plotter = ProjectPlotter()
    p_plotter.plot_avg_methods("samples/all-avg5-ascending-sk-log-10000.csv")
