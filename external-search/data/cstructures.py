""" <cstructures.py>

    Defines data-classes representing structures in the C programs."""


from typing import Callable, Any
from dataclasses import dataclass
from abc import ABC, abstractmethod


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
