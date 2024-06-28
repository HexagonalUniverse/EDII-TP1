""" <project_manager.py>

    The external-search manager. Defines a class for interacting with it. """

from subprocess import CompletedProcess


class ProjectManager(object):

    """A class with the purpose of interfacing at high level the interactions with the project. Those may be
    "build", "run", "generate" calls, etc. """

    __slots__: list[str] = [
        "__r_paths", "__a_paths"
    ]

    def __getitem__(self, item: str) -> str:
        return self.__a_paths[item]

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

    def run_process_in_context(self, * args: str, timeout: int = 1) -> CompletedProcess:
        import subprocess

        # print("process args:", args)

        process_result: CompletedProcess
        process_result = subprocess.run(
            args, stdout=subprocess.PIPE, text=True, timeout=timeout, shell=False, cwd=self.__a_paths["base_dir"]
        )

        if process_result.returncode != 0:
            raise RuntimeError
        return process_result

    def generate_input(self, *, registries_qtt: int = 100, file_order: str = "unordered") -> CompletedProcess:
        """Generate registries data to be used on <pesquisa.exe>. Wrapper around <data-gen.exe> call, with the output as
         the input file for it."""

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
        timeout: int = 180

        return self.run_process_in_context(
            self.__a_paths["data_gen_executable"], str(file_order_id), str(registries_qtt),
            self.__a_paths["input_datafile"], timeout=timeout
        )

    def rebuild(self, * compilation_directives: str, debug_mode: str | None = None,
                transparent_mode: bool = True) -> CompletedProcess:

        the_debug_modes: list[None | str] = [None, "log", "stderr"]
        if debug_mode is not None:
            if not isinstance(debug_mode, str):
                raise TypeError
            elif debug_mode not in the_debug_modes:
                raise ValueError

        if not isinstance(transparent_mode, bool):
            raise TypeError

        makefile_directives: list[str] = [
            "DEBUG=" + str(the_debug_modes.index(debug_mode)),
            "TRANSPARENT=" + ("1" if transparent_mode else "0"),
            "OPTIONAL_COMPILE_DIRECTIVES=" + (
                "" if not compilation_directives else f"\"{
                    " ".join(compilation_directives)
                }\""
            )
        ]

        timeout: int = 2    # seconds
        return self.run_process_in_context("make", "rebuild", * makefile_directives, timeout=timeout)


if __name__ == "__main__":
    pm = ProjectManager()
    pm.rebuild(debug_mode=None, transparent_mode=True)
