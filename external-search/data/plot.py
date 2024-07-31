""" <plot.py>

    Graphs the project results. Bases on matplotlib."""


import matplotlib
from matplotlib import pyplot as plt
from abc import abstractmethod
from project_manager import ProjectManager
from typing import IO, Callable


def lmap(f: Callable, l: list) -> list:
    return list(map(f, l))


class GraphRenderer(object):
    """An OOP abstraction for rendering graphs, based on two steps: building, and rendering.
        It is such that, choosing whether to render a matplotlib graph with the default showing mode,
        or saving it as a PGF file separately, implies in no more than signalizing a boolean."""
    
    __slots__: list[str] = [
        "__output_dir", "__pgf", "__default_mpl_backend", "_figures", "_fig_names"
    ]

    def __init__(self, __output_dir: str) -> None:
        self.__output_dir: str = __output_dir

        # Tracks whether the PGF context was set in matplotlib.
        self.__pgf: bool = False

        self.__default_mpl_backend: str = matplotlib.get_backend()
        
        self._figures: list = []
        self._fig_names: list[str] = []
    
    def __render(self, __filename: str = "undefined.pgf") -> None:
        if not self.__pgf:
            plt.show()
            return None
        
        if self._figures:
            for fig, filename in zip(self._figures, self._fig_names):
                fig.savefig(self.__output_dir + filename, format="pgf")
            return None
        
        plt.savefig(self.__output_dir + __filename, format="pgf")
        plt.clf()
        return None

    def __set_PGF_context(self) -> None:
        """Sets the PGF context such that all subsequent plots will be rendered as PGF files."""

        matplotlib.use("pgf")
        matplotlib.rcParams.update({
            "pgf.texsystem": "pdflatex",
            'font.family': "serif",
            'text.usetex': True,
            'pgf.rcfonts': True,
        })
        # As it seems the following parameter yields LaTeX errors:
        # 'text.usetex': True,

        self.__pgf = True

    def __unset_PGF_context(self) -> None:
        """Undoes __set_PGF_context: sets the matplotlib backend back to its original state."""
        matplotlib.use(self.__default_mpl_backend)
        self.__pgf: bool = False
    
    def __update_PGF_context(self, pgf: bool) -> None:
        if not self.__pgf and pgf:
            self.__set_PGF_context()
        elif self.__pgf and not pgf:
            self.__unset_PGF_context()
    
    @abstractmethod
    def graph(self) -> None:
        """Assembles the graph (based on matplotlib's API)."""
        from random import random

        plt.title("Blank")
        plt.plot([i for i in range(100)], [random() for _ in range(100)])
    
    def render(self, pgf: bool = False) -> None:
        self.__update_PGF_context(pgf)
        
        # plt.clf()       # Cleaning pyplot's screen buffer.
        self.graph()
        
        self.__render()


class PlotAvgLogMethods(GraphRenderer):
    def __init__(self, read_filename: str, output_dir: str = "pgfs/") -> None:
        self.r_filename: str = read_filename
        super().__init__(output_dir)

        self.__parse_data()
        self.__preprocess_data()
        
        self.curve_colors: dict[str, str] = {
            "ISS"   :   "#FF8BFF",
            "EBST"  :   "#DEA14E",
            "B"     :   "#FE6363",
            "B*"    :   "#4E9DDE",
        }
         
        self.alpha_values: dict[str, int] = {
            "ISS"   :   0.75,
            "EBST"  :   0.75,
            "B"     :   0.75,
            "B*"    :   0.75,
        }
    
    def __parse_data(self) -> None:
        method: int
        samples: int
        card: int
        transparent: list[int]
        c_time: float
        s_time: float
        
        parsed_data: list[list[int | float]] = []
        
        data_file: IO = open(self.r_filename, "r")
        data_file.readline()
        for line in data_file:
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            parsed_data.append([method, samples, card, transparent, c_time, s_time])
            print(parsed_data[-1])
        data_file.close()

        self.data: dict[str, list[int | float]] = {
            "ISS":  list(filter(lambda l: l[0] == 0, parsed_data)),
            "EBST": list(filter(lambda l: l[0] == 1, parsed_data)),
            "B":    list(filter(lambda l: l[0] == 2, parsed_data)),
            "B*":   list(filter(lambda l: l[0] == 3, parsed_data)),
        }
        
        # The entries dimension - n, where n is the number of registries in each sample - is pretty much the same for any of them.
        self.entries_dimension: list[int] = [__list[2] for __list in self.data["B"]]
        
        # Also, the number of samples is assumed to be the same for all of them.
        self.samples: int = self.data["B"][0][1]
    
    def __preprocess_data(self) -> None:        
        self.d_construction_time: dict[str, list[float]] = dict()
        for key in self.data:
            self.d_construction_time[key] = lmap(lambda l: 1000 * l[-2], self.data[key])
    
        self.d_searching_time: dict[str, list[float]] = dict()
        for key in self.data:
            self.d_searching_time[key] = lmap(lambda l: l[-1], self.data[key])
    
        self.d_construction_cmp: dict[str, list[int]] = dict()
        for key in self.data:
            self.d_construction_cmp[key] = lmap(lambda l: l[3][-1], self.data[key])

        self.d_searching_cmp: dict[str, list[int]] = dict()
        for key in self.data:
            self.d_searching_cmp[key] = lmap(lambda l: l[3][-2], self.data[key])

        self.d_total_io: dict[str, list[int]] = dict()
        for key in self.data:
            self.d_total_io[key] = lmap(lambda l: sum(l[3][:-2]), self.data[key])
        
        self.d_regpage_io: dict[str, list[int]] = dict()
        for key in self.data:
            self.d_regpage_io[key] = lmap(lambda l: sum(l[3][0:2]), self.data[key])
    
        self.d_self_io: dict[str, list[int]] = dict()
        self.d_self_io["ISS"] = lmap(lambda l: sum(l[3][0:2]), self.data["ISS"])
        self.d_self_io["EBST"] = lmap(lambda l: sum(l[3][2:4]), self.data["EBST"])
        self.d_self_io["B"] = lmap(lambda l: sum(l[3][6:8]), self.data["B"])
        self.d_self_io["B*"] = lmap(lambda l: sum(l[3][8:10]), self.data["B*"])

    def __dual_graph(self, y1: dict[str, list[int | float]], y2: dict[str, list[int | float]], log_x: bool = True):
        fig = plt.figure(figsize=(18, 8))
        spec = fig.add_gridspec(1, 2)
        axes: list = [fig.add_subplot(spec[i // 2, i % 2]) for i in range(2)]
            
        for key in self.data:
            axes[0].plot(self.entries_dimension, y1[key], alpha=self.alpha_values[key], label=key, color=self.curve_colors[key])
            
        for key in self.data:
            axes[1].plot(self.entries_dimension, y2[key], alpha=self.alpha_values[key], label=key, color=self.curve_colors[key])
        
        # Stylizing
        for axis in axes:
            axis.grid()
            axis.legend()
            axis.set_xlabel("$N$: Registries Qtt [u.]")

            if log_x:
                axis.set_xscale("log")
            
        fig.suptitle(f"Avg. {self.samples} Log-Sampling $10 \\to 10^6$\nfor External Searching Data Structures")
        return fig, axes

    def __common_graph(self, y: dict[str, list[int | float]], log_x: bool = True):
        fig = plt.figure(figsize=(8,8))
        axis = fig.add_subplot()
        
        for key in self.data:
            axis.plot(self.entries_dimension, y[key], alpha=self.alpha_values[key], label=key, color=self.curve_colors[key])

        # Stylizing
        axis.grid()
        axis.legend()
        axis.set_xlabel("$N$: Registries Qtt [u.]")
        
        if log_x:
            axis.set_xscale("log")
            
        fig.suptitle(f"Avg. {self.samples} Log-Sampling $10 \\to 10^6$\nfor External Searching Data Structures")
        return fig, axis

    def graph(self, log_x: bool = True) -> None:
        
        # Construction Time
        if True:
            fig, axis = self.__common_graph(self.d_construction_time, log_x)

            axis.set_yscale("log")
            axis.set_title("$(\\log \\times \\log)$ Construction Time")
            axis.set_ylabel("$T_C$: Construction Time [ms]")

            self._figures.append(fig)
            self._fig_names.append("avglog-construction-time.pgf")

        # Searching Time
        if True:    
            fig, axis = self.__common_graph(self.d_searching_time, log_x)
            
            axis.set_title("$(\\log \\times \\mathtt{lin})$ Searching Time")
            axis.set_ylabel("$T_S$: Searching Time [ms]")

            self._figures.append(fig)
            self._fig_names.append("avglog-searching-time.pgf")
                    
        # Construction Comparisons
        if True:
            fig, axis = self.__common_graph(self.d_construction_cmp, log_x)
            
            axis.set_yscale("log")
            axis.set_ylabel("$\\phi_C$: Comparison Count in Construction [u.]")
            axis.set_title("$(\\log \\times \\log)$ Comparisons Counting in the Contruction")

            self._figures.append(fig)
            self._fig_names.append("avglog-construction-cmp.pgf")
           
        # Searching Comparisons
        if True:
            fig, axis = self.__common_graph(self.d_searching_cmp, log_x)

            axis.set_title("$(\\log \\times \\mathtt{lin})$ Comparisons Counting in the Search")
            axis.set_ylabel("$\\phi_S$: Comparison Count in Searching [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglog-searching-cmp.pgf")
        
        # Searching Comparisons (w/o ISS)
        if True:
            fig = plt.figure(figsize=(8,8))
            axis = fig.add_subplot()
                
            for key in self.data:
                if key == "ISS":
                    continue
                axis.plot(self.entries_dimension, self.d_searching_cmp[key], alpha=self.alpha_values[key], label=key, color=self.curve_colors[key])
            
            if log_x:
                axis.set_xscale("log")

            # Stylizing
            axis.grid()
            axis.legend()
            axis.set_xlabel("$N$: Registries Qtt [u.]")
            
            fig.suptitle(f"Avg. {self.samples} Log-Sampling $10 \\to 10^6$\nfor External Searching Data Structures")
            
            axis.set_title("$(\\log \\times \\mathtt{lin})$ Comparisons Counting in the Search (w/o ISS)")
            axis.set_ylabel("$\\phi_S$: Comparison Count in Searching [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglog-searching-cmp2.pgf")

        # Total IO Transferences
        if True:
            fig, axis = self.__common_graph(self.d_total_io, log_x)
            
            axis.set_yscale("log")
            axis.set_title("$(\\log \\times \\log)$ Total IO Page Transferences")
            axis.set_ylabel("$\\xi$: (Total) Page Transferences [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglog-total-io.pgf")

        # Total Registries-Page IO Transferences
        if True:
            fig, axis = self.__common_graph(self.d_regpage_io, log_x)
            
            axis.set_title("$(\\log \\times \\mathtt{lin})$ Total Registries-Page IO Transferences")
            axis.set_ylabel("$\\xi_{\\mathcal{R}}$: (Total) Registries-page Transferences [u.]")
            
            self._figures.append(fig)
            self._fig_names.append("avglog-total-reg-io.pgf")
            
        # Self IO Transferences
        if True:
            fig, axis = self.__common_graph(self.d_self_io, log_x)
            
            axis.set_title("$(\\log \\times \\mathtt{lin})$ Self-IO Page Transferences")
            axis.set_ylabel("$\\xi$: (Self) Page Transferences [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglog-selfio.pgf")
            
        return None


class PlotEbstErbt(GraphRenderer):
    def __init__(self, read_filename: str, output_dir: str = "pgfs/") -> None:
        self.r_filename: str = read_filename
        super().__init__(output_dir)

        self.__parse_data()
        self.__preprocess_data()
        
        self.curve_colors: dict[str, str] = {
            "EBST"  :   "#FF8BFF",
            "ERBT"  :   "#DEA14E",
        }
        
        self.alpha_values: dict[str, str] = {
            "EBST"  :  0.75,
            "ERBT"  :   0.75,
        }

        self._fig_size: tuple[int, int] = (8, 8)

    def __parse_data(self) -> None:
        card: int
        transparent: list[int]
        c_time: float
        s_time: float

        data_file: IO = open(self.r_filename, "r")
        data_file.readline()
        data_file.readline()

        self.ebst_data: list = []
        for line in data_file:
            if line == "erbt\n":
                break
            line_values: list[str] = line.split(",")
            _, _, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            self.ebst_data.append([card, transparent, c_time, s_time])

        self.erbt_data: list = []
        for line in data_file:
            line_values: list[str] = line.split(",")
            _, _, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            self.erbt_data.append([card, transparent, c_time, s_time])
        data_file.close()
    
    def __preprocess_data(self) -> None:

        # Note that the resolution of sampling is bigger for the ebst
        self.entries_dimension_ebst: list[int] = lmap(lambda l: l[0], self.ebst_data)
        self.entries_dimension_erbt: list[int] = lmap(lambda l: l[0], self.erbt_data)
        
        self.d_ebst_total_cmp: list[int] = lmap(lambda l: sum(l[1][-2:]), self.ebst_data)
        self.d_erbt_total_cmp: list[int] = lmap(lambda l: sum(l[1][-2:]), self.erbt_data)
    
        self.d_erbt_construction_time_ratio: list[float] = list()
        self.d_erbt_searching_time_ratio: list[float] = list()
        for i, x in enumerate(self.entries_dimension_erbt):
            j: int = self.entries_dimension_ebst.index(x)

            self.d_erbt_construction_time_ratio.append(
                self.erbt_data[i][-2] / self.ebst_data[j][-2]
            )

            self.d_erbt_searching_time_ratio.append(
                self.erbt_data[i][-1] / self.ebst_data[j][-1]    
            )
        
        self.d_ebst_total_io: list[int] = lmap(lambda l: sum(l[1][:-2]), self.ebst_data)
        self.d_erbt_total_io: list[int] = lmap(lambda l: sum(l[1][:-2]), self.erbt_data)
        
        self.d_ebst_searching_time: list[float] = lmap(lambda l: l[-1] * (10 ** 6), self.ebst_data)
        self.d_erbt_searching_time: list[float] = lmap(lambda l: l[-1] * (10 ** 6), self.erbt_data)

        self.d_ebst_scmp: list[int] = lmap(lambda l: l[1][10], self.ebst_data)
        self.d_erbt_scmp: list[int] = lmap(lambda l: l[1][10], self.erbt_data)

    def __common_double_graph(self, y1: tuple[str, list[int | float]], y2: tuple[str, list[int | float]]):
        fig = plt.figure(figsize=self._fig_size)
        axis = fig.add_subplot()
        
        axis.plot(self.entries_dimension_ebst, y1, alpha=self.alpha_values["EBST"], label="EBST (MRT)", color=self.curve_colors["EBST"])
        axis.plot(self.entries_dimension_erbt, y2, alpha=self.alpha_values["ERBT"], label="ERBT", color=self.curve_colors["ERBT"])

        axis.grid()
        axis.legend()
        axis.set_xlabel("$N$: Registries Qtt [u.]")
        
        fig.suptitle(f"Avg. 5 Lin-Sampling $1000 \\to 10^5$\nfor External Searching Data Structures")
        return fig, axis

    def graph(self) -> None:
        # Total of Comparisons (Y linear)
        if True:
            fig, axis = self.__common_double_graph(self.d_ebst_total_cmp, self.d_erbt_total_cmp)
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Total Comparisons")
            axis.set_ylabel("$\\phi$: Total of Comparisons [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-lin-cmp.pgf")
        
        # Total of Comparisons (Y logarithmic)
        if True:    
            fig, axis = self.__common_double_graph(self.d_ebst_total_cmp, self.d_erbt_total_cmp)
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{log})$ Total Comparisons")
            axis.set_yscale("log")
            axis.set_ylabel("$\\phi$: Total of Comparisons [u.]")
        
            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-log-cmp.pgf")
        
        # Searching Comparisons
        if True:
            fig, axis = self.__common_double_graph(self.d_ebst_scmp, self.d_erbt_scmp)
            
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{log})$ Searching Comparisons")
            axis.set_ylabel("$\\phi_S$: Searching Comparisons [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-scmp.pgf")
        
        # Construction Time Ratio
        if True:
            fig = plt.figure(figsize=self._fig_size)
            axis = fig.add_subplot()

            axis.plot(self.entries_dimension_erbt, self.d_erbt_construction_time_ratio,
                      alpha=1, label="$\\kappa = \\frac{T_C\\{\\mathcal{N}\\}}{T_C\\{\\mathcal{E}\\}}$", color="#4E9DDE")

            axis.grid()
            axis.legend()
            axis.set_xlabel("$N$: Registries Qtt [u.]")
            axis.set_ylabel("$\\kappa_C$: Ratio of Construction Times [u.]")
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Construction Time Ratio")
            fig.suptitle(f"Avg. 5 Lin-Sampling $1000 \\to 10^5$\nfor External Searching Data Structures")

            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-construction-time-ratio.pgf")

        # Searching Time
        if True:
            fig, axis = self.__common_double_graph(self.d_ebst_searching_time, self.d_erbt_searching_time)
            axis.set_ylabel("$T_S$: Searching Time [$\\mu$s]")
            axis.set_title("($\\mathtt{lin} \\times \\mathtt{lin}$) Searching Time")
            
            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-searching-time.pgf")
        
        # Searching Time Ratio
        if True:    
            fig = plt.figure(figsize=self._fig_size)
            axis = fig.add_subplot()
            
            axis.plot(self.entries_dimension_erbt, self.d_erbt_searching_time_ratio,
                      alpha=1, label="$\\kappa = \\frac{T_S\\{\\mathcal{N}\\}}{T_S\\{\\mathcal{E}\\}}$", color="#4E9DDE")

            axis.grid()
            axis.legend()
            axis.set_xlabel("$N$: Registries Qtt [u.]")
            axis.set_ylabel("$\\kappa_S:$ Ratio of Searching Times [u.]")
            fig.suptitle(f"Avg. 5 Lin-Sampling $1000 \\to 10^5$\nfor External Searching Data Structures")

            self._figures.append(fig)
            self._fig_names.append("avglin-EBSTs-searching-time-ratio.pgf")
        
        # Total IO Transferences
        if False:
            # fig, axis = self.__common_double_graph(self.d_ebst_total_io, self.d_erbt_total_io)
            # axis.set_yscale("log")
            # axis.set_title("$(\\log \\times \\log)$ Total IO Page Transferences")
            # axis.set_ylabel("$\\xi$: (Total) Page Transferences")
            fig = plt.figure(figsize=self._fig_size)
            axis = fig.add_subplot()
            axis.plot(self.entries_dimension_ebst, self.d_ebst_total_io, alpha=0.1)
            axis.plot(self.entries_dimension_erbt, self.d_erbt_total_io, alpha=0.5)


class PlotBTrees(GraphRenderer):
    def __init__(self, read_filename: str, output_dir: str = "pgfs/", page_buffersize: int = None) -> None:
        self.r_filename: str = read_filename
        super().__init__(output_dir)

        self.__parse_data()
        self.__preprocess_data()
        
        self.curve_colors: dict[str, str] = {
            "B"     :   "#FF8BFF",
            "BS"    :   "#DEA14E",
        }
        
        self.alpha_values: dict[str, str] = {
            "B"     :   0.75,
            "BS"    :   0.75,
        }

        self._fig_size: tuple[int, int] = (8, 8)

        if page_buffersize is None:
            self.figures_title: str = "Avg. 5\tB / B* $(t = 21)$"
        
        else:
            t_b: int = (page_buffersize - 8) // 12
            t_bs: int = (page_buffersize - 8) // 8

            self.figures_title: str = "Avg. 5\tB / B* $(t_{\\beta} = " + str(t_b) + ", t_{\\beta^{*}} = " + str(t_bs) + ")$"

    def __parse_data(self) -> None:
        method: int
        
        samples: int
        card: int
        transparent: list[int]
        c_time: float
        s_time: float

        data_file: IO = open(self.r_filename, "r")
        data_file.readline()
        data_file.readline()

        self.b_data: list = []
        self.bs_data: list = []
        for line in data_file:
            line_values: list[str] = line.split(",")
            method, samples, card, * transparent = map(int, line_values[:-2])
            c_time, s_time = map(float, line_values[-2:])
            
            if method == 2:
                self.b_data.append([card, transparent, c_time, s_time])
            else:
                self.bs_data.append([card, transparent, c_time, s_time])
        
        data_file.close()

    def __preprocess_data(self) -> None:
        
        self.entries_b: list[int] = lmap(lambda l: l[0], self.b_data)
        self.entries_bs: list[int] = lmap(lambda l: l[0], self.bs_data)

        self.d_construction_cmp_b: list[int] = lmap(lambda l: l[1][-1], self.b_data)
        self.d_construction_cmp_bs: list[int] = lmap(lambda l: l[1][-1], self.bs_data)

        self.d_searching_cmp_b: list[int] = lmap(lambda l: 1000 * l[1][-2], self.b_data)
        self.d_searching_cmp_bs: list[int] = lmap(lambda l: 1000 * l[1][-2], self.bs_data)

        self.d_construction_time_b: list[float] = lmap(lambda l: l[-2], self.b_data)
        self.d_construction_time_bs: list[float] = lmap(lambda l: l[-2], self.bs_data)

        self.d_searching_time_b: list[float] = lmap(lambda l: l[-1], self.b_data)
        self.d_searching_time_bs: list[float] = lmap(lambda l: l[-1], self.bs_data)

        self.d_total_IO_transferences_b: list[int] = lmap(lambda l: sum(l[1][:-2]), self.b_data)
        self.d_total_IO_transferences_bs: list[int] = lmap(lambda l: sum(l[1][:-2]), self.bs_data)
        
        self.d_b_read: list[int] = lmap(lambda l: l[1][6], self.b_data)
        self.d_bs_read: list[int] = lmap(lambda l: l[1][8], self.bs_data)

        self.d_b_write: list[int] = lmap(lambda l: l[1][7], self.b_data)
        self.d_bs_write: list[int] = lmap(lambda l: l[1][9], self.bs_data)

        print("Avg. B / B* searching time: ", 
              sum(self.d_searching_time_b) / len(self.entries_b),
              sum(self.d_searching_time_bs) / len(self.entries_bs))

    def __common_dual_graph(self, y_b: list[int | float], y_bs: list[int | float]):
        fig = plt.figure(figsize=(8,8))
        axis = fig.add_subplot()
        
        axis.plot(self.entries_b, y_b, alpha=self.alpha_values["B"], label="B", color=self.curve_colors["B"])
        axis.plot(self.entries_bs, y_bs, alpha=self.alpha_values["BS"], label="B*", color=self.curve_colors["BS"])    

        # Pre-stylizing
        axis.grid()
        axis.legend()
        axis.set_xlabel("$N$: Registries Qtt [u.]")
        
        fig.suptitle(self.figures_title)
        return fig, axis

    def graph(self) -> None:
        
        # Construction Comparisons
        if True:
            fig, axis = self.__common_dual_graph(self.d_construction_cmp_b, self.d_construction_cmp_bs)
           
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Avg. Comparisons in Construction")
            axis.set_ylabel("$\\phi_C$: Construction Comparisons [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglinb-construction-cmp.pgf")
        
        # Searching Comparisons
        if True:
            fig, axis = self.__common_dual_graph(self.d_searching_cmp_b, self.d_searching_cmp_bs)
            
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Avg. Comparisons in Searching")
            axis.set_ylabel("$\\phi_S$: Searching Comparisons [u.]")
            
            self._figures.append(fig)
            self._fig_names.append("avglinb-searching-cmp.pgf")
        
        # Construction Time
        if True:
            fig, axis = self.__common_dual_graph(self.d_construction_time_b, self.d_construction_time_bs)    

            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Avg. Time took in Construction")
            axis.set_ylabel("$T_C$: Construction Time [s]")

            self._figures.append(fig)
            self._fig_names.append("avglinb-construction-time.pgf")
        
        # Searching Time
        if True:
            fig, axis = self.__common_dual_graph(self.d_searching_time_b, self.d_searching_time_bs)

            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Avg. Time took in Searching")
            axis.set_ylabel("$T_S$: Searching Time [ms]")
            
            self._figures.append(fig)
            self._fig_names.append("avglinb-searching-time.pgf")
        
        # Self Read Transferences
        if True:
            fig, axis = self.__common_dual_graph(self.d_b_read, self.d_bs_read)
            
            axis.set_title("$\\mathtt{lin} \\times \\mathtt{lin}$ Avg. Number of B / B* Pages Reading")
            axis.set_ylabel("$\\xi_I$: Read Transferences [u.]")
            
            self._figures.append(fig)
            self._fig_names.append("avglinb-read.pgf")

        # Self Write Transferences
        if True:
            fig = plt.figure(figsize=(18,8))
            spec = fig.add_gridspec(1, 2)
            axes = [fig.add_subplot(spec[0, 0]), fig.add_subplot(spec[0, 1])]

            axes[0].plot(self.entries_b, [x / y for x, y in zip(self.d_b_write, self.d_bs_write)], label="$\\xi_O\\{B\\} - \\xi_O\\{B^{*}\\}$ (Write)", color="#34B8FF")
            axes[1].plot(self.entries_b, [x - y for x, y in zip(self.d_b_write, self.d_bs_write)], label="$\\xi_O\\{B\\} - \\xi_O\\{B^{*}\\}$ (Write)", color="#34B8FF")

            # Style
            for axis in axes:
                axis.grid()
                axis.legend()
                axis.set_xlabel("$N$: Registries Qtt [u.]")
            
            fig.suptitle(self.figures_title)

            axes[0].set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Ratio Between B / B* Pages Writing")
            axes[0].set_ylabel("$\\kappa_O$: Write Transferences Ratio [u.]")
            
            axes[1].set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Difference Between B / B* Pages Writing")
            axes[1].set_ylabel("Write Transferences Difference [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglinb-write.pgf")

        # Total IO Transferences
        if True:
            fig, axis = self.__common_dual_graph(self.d_total_IO_transferences_b, self.d_total_IO_transferences_bs)
            
            axis.set_title("$(\\mathtt{lin} \\times \\mathtt{lin})$ Avg. Number of Total IO Transferecnes")
            axis.set_ylabel("$\\xi$: Total Transferences [u.]")

            self._figures.append(fig)
            self._fig_names.append("avglinb-total-io.pgf")


if __name__ == "__main__":
    
    if False:
        avg_log_plot = PlotAvgLogMethods("samples/archived/all-avg5-ascending-sk-log-1000000.csv")
        avg_log_plot.render(False)
    
    if False:
        avg_ebsts_plot = PlotEbstErbt("samples/archived/ebst_erbt-avg5-lin-100000.csv")
        avg_ebsts_plot.render(True)
    
    if True:
        if False:
            avg_btrees_plot = PlotBTrees("samples/archived/bb+-avg5-lin-100000.csv")
            avg_btrees_plot.render(True)

        if True:
            avg_btrees_plot = PlotBTrees("samples/archived/bbs-avg5-lin-100000-1024.csv", output_dir="pgfs/bseq/bseq-",
                                         page_buffersize = 1024)
            avg_btrees_plot.render(True)

    # p_plotter = ES_Plotter()
    # p_plotter.plot_avg_methods("samples/archived/all-avg5-ascending-sk-log-1000000.csv")
    # p_plotter.plot_erbt_ebst("samples/archived/ebst_erbt-avg5-lin-100000.csv")
    # p_plotter.plot_b_bplus("samples/archived/bb+-avg5-lin-100000.csv")
    