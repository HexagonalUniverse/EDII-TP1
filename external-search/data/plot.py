""" <plot.py>

    Graphs the project results. Bases on matplotlib."""


import matplotlib
from matplotlib import pyplot as plt
from abc import abstractmethod
from project_manager import ProjectManager
from typing import IO


class ES_Plotter(object):
    """ External Search Plotter
    =========================== """

    def __init__(self) -> None:
        self.__project_manager: ProjectManager = ProjectManager()

    def plot_avg_methods(self, r_filename: str) -> None:
        # Parsing the Data
        # ================
        
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

        class ArvoreBinaria:
            class Nothing:
                class ABC:
                    ...
        
        data: dict[str, list[int | float]] = {
            "ISS":  list(filter(lambda l: l[0] == 0, parsed_data)),
            "EBST": list(filter(lambda l: l[0] == 1, parsed_data)),
            "B":    list(filter(lambda l: l[0] == 2, parsed_data)),
            "B+":   list(filter(lambda l: l[0] == 3, parsed_data)),
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

        # Time Graph
        # ----------
        
        fig = plt.figure(figsize=(20, 8))
        spec = fig.add_gridspec(1, 2)
        axes: list = [fig.add_subplot(spec[i // 2, i % 2]) for i in range(2)]
        
        for key in data:
            for __list in data[key]:
                print(f"{key}: {__list}")
        
        methods_colors: dict[str, str] = {
            "ISS"   :   "#FF8BFF",
            "EBST"  :   "#DEA14E",
            "B"     :   "#FE6363",
            "B+"    :   "#4E9DDE",
        }

        # axes[0]: Construction Time.
        for key in data:
            axes[0].plot(entries_dimension, [1000 * __list[-2] for __list in data[key]], alpha=1, label=key, color=methods_colors[key])
        
        # axes[1]: Searching Time.
        for key in data:
            axes[1].plot(entries_dimension, [1000 * __list[-1] for __list in data[key]], alpha=1, label=key, color=methods_colors[key])
        
        for axis in axes:   # Stylizing
            axis.grid()
            axis.legend(list(data))
            axis.set_xlabel("# Registries [u.]")
            axis.set_xscale("log")
        
        fig.suptitle("Average (5) log-sampling\nfor Searching Data Structures")

        axes[0].set_yscale("log")
        axes[0].set_title("(log x log) Construction Time")
        axes[0].set_ylabel("Construction Time [ms]")

        axes[1].set_title("(log x lin) Searching Time")
        axes[1].set_ylabel("Searching Time [ms]")

        plt.show()  # Displaying it

        # Comparisons Graph
        # -----------------

        fig = plt.figure(figsize=(20, 10))
        spec = fig.add_gridspec(1, 2)
        axes: list = [fig.add_subplot(spec[i // 2, i % 2]) for i in range(2)]
        
        # axes[0]: Comparisons in build.
        for key in data:
            axes[0].plot(entries_dimension, [__list[3][-2] for __list in data[key]], alpha=0.5, label=key)

        # axes[1]: Comparisons in search.
        for key in data:
            axes[1].plot(entries_dimension, [__list[3][-1] for __list in data[key]], alpha=0.5, label=key)
        
        for axis in axes:   # Stylizing
            axis.grid()
            axis.legend(list(data))
            axis.set_xlabel("# Registries [u.]")
            axis.set_xscale("log")

        axes[0].set_yscale("log")
        axes[0].set_title("(log x log) Comparisons Counting in Build")
        axes[0].set_ylabel("Comparison Count in Building")

        # axes[3].set_yscale("log")
        axes[1].set_title("(log x log) Comparisons Counting in Search")
        axes[1].set_ylabel("Comparison Count in Searching")

        plt.show()  # Displaying it       

        # Total IO Graph
        # --------------

        fig = plt.figure(figsize=(20, 10))
        spec = fig.add_gridspec(1, 1)
        axes: list = fig.subplots(spec[0, 0])
        
        # axes[4]: Total IO transferences.
        for key in data:
            axes[0].plot(entries_dimension, [sum(__list[3][:-2]) for __list in data[key]], alpha=0.5, label=key)
        
        axes[0].set_yscale("log")
        axes[0].set_title("(log x lin) Total IO Page Transference")
        axes[0].set_ylabel("Total Page Transference")

        # Displaying it
        plt.show()
        return None

    def plot_erbt_ebst(self, r_filename: str) -> None:

        # Parsing the data
        # ----------------

        method: int
        samples: int
        card: int
        transparent: list[int]
        c_time: float
        s_time: float

        data_file: IO = open(r_filename, "r")
        data_file.readline()
        data_file.readline()

        ebst_data: list = []
        for line in data_file:
            if line == "erbt\n":
                break
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            ebst_data.append([card, transparent, c_time, s_time])

        erbt_data: list = []
        for line in data_file:
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            erbt_data.append([card, transparent, c_time, s_time])
        data_file.close()

        # Filtering... (No longer needed)
        # ebst_data = list(filter(lambda x: 0_000 <= x[0] <= 100_000, ebst_data))
        # erbt_data = list(filter(lambda x: 0_000 <= x[0] <= 100_000, erbt_data))

        # Plotting the values
        # -------------------

        # Note that the resolution of sampling is bigger for the ebst
        entries_dimension_ebst: list[int] = [__list[0] for __list in ebst_data]
        
        entries_dimension_erbt: list[int] = [__list[0] for __list in erbt_data]
        plt.plot(entries_dimension_ebst, [sum(__list[1][-1:]) for __list in ebst_data], c="#0000FF")
        plt.plot(entries_dimension_erbt, [sum(__list[1][-1:]) for __list in erbt_data], c="#FF0000")

        # plt.yscale("log")

        plt.show()

        # ~~ second graph

        ratio_data: list = []
        for j, x in enumerate(entries_dimension_erbt):
            ratio_data.append(
                erbt_data[j][-2] / ebst_data[entries_dimension_ebst.index(x)][-2]
            )

        plt.plot(entries_dimension_erbt, ratio_data)
        plt.show()

    def plot_b_bplus(self, r_filename: str) -> None:
        
        # Parsing the data
        # ----------------

        method: int
        samples: int
        card: int
        transparent: list[int]
        c_time: float
        s_time: float

        data_file: IO = open(r_filename, "r")
        data_file.readline()
        data_file.readline()

        b_data: list = []
        bplus_data: list = []
        for line in data_file:
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            
            if method == 2:
                b_data.append([card, transparent, c_time, s_time])
            else:
                bplus_data.append([card, transparent, c_time, s_time])
        
        data_file.close()
        
        entries_b: list[int] = [__list[0] for __list in b_data]
        entries_bs: list[int] = [__list[0] for __list in bplus_data]
        
        plt.plot(entries_b, [sum(__list[1][:-2]) for __list in b_data], alpha=0.5, c="#FF0000")
        plt.plot(entries_bs, [sum(__list[1][:-2]) for __list in bplus_data], alpha=0.5, c="#0000FF")
        plt.show()
        
        plt.plot(entries_b, [__list[-2] for __list in b_data], alpha=0.5, c="#FF0000")
        plt.plot(entries_bs, [__list[-2] for __list in bplus_data], alpha=0.5, c="#0000FF")
        plt.show()

        plt.plot(entries_b, [__list[-1] for __list in b_data], alpha=0.5, c="#FF0000")
        plt.plot(entries_bs, [__list[-1] for __list in bplus_data], alpha=0.5, c="#0000FF")
        plt.show()

        plt.plot(entries_b, [__list[1][-2] + __list[1][-1] for __list in b_data], alpha=0.5, c="#FF0000")
        plt.plot(entries_bs, [__list[1][-2] + __list[1][-1] for __list in bplus_data], alpha=0.5, c="#0000FF")
        plt.show()


if __name__ == "__main__":
    p_plotter = ES_Plotter()
    p_plotter.plot_avg_methods("samples/archived/all-avg5-ascending-sk-log-1000000.csv")
    # p_plotter.plot_erbt_ebst("samples/archived/ebst_erbt-avg5-lin-100000.csv")
    # p_plotter.plot_b_bplus("samples/archived/bb+-avg5-lin-100000.csv")
    