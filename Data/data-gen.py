"""
    A script to generate - and write out - the data about being used in
the external search methods in the programs.

    Research material:

https://www.digitalocean.com/community/tutorials/python-struct-pack-unpack
https://docs.python.org/3.7/library/struct.html#format-characters
https://stackoverflow.com/questions/9940859/fastest-way-to-pack-a-list-of-floats-into-bytes-in-python

About context managers:
    https://builtin.com/software-engineering-perspectives/what-is-with-statement-python
    
   
"""


from struct import pack
from dataclasses import dataclass



alpha_numericals: list[str] = [chr(i) for i in range(65, 91)] + [chr(i) for i in range(97, 123)] + [chr(i) for i in range(48, 58)]


@dataclass(frozen=True)
class Constants(object):
    """
        Where the script constants are all stored.
    """

    def __init__(self) -> None:
        # This object shall not be instantiated. While it does not happens of me finding a better 
        # method of imposing that restriction, an RuntimeError is raised at __init__.
        raise RuntimeError

    data_2_string_length: int = 1_000
    data_3_string_length: int = 5_000

    data_2_pack_fstr: str = f"{data_2_string_length}s"
    data_3_pack_fstr: str = f"{data_3_string_length}s"

    INT_MAX: int = 2 ** 31 - 1
    INT_MIN: int = - 2 ** 31



def fullfill_string_bytes(__String: str, __length: int, 
                        __format: str = "utf-8", __filling: bytes = b"\0") -> bytes:
    """Returns a byte-string of size "__length" with its beggining matching the string 
    encoding and the rest of it being fulfilled with "__filling". By default, encodes the
    string in utf-8 and fulfills the byte-string with the null character.
    """

    __string_length: int = len(__String)
        
    # Necessarily the specified length has to be bigger or equal the string length.
    if __length < __string_length:
        raise ValueError

    return pack(
        f"{__string_length}s", bytes(__String, __format)
    ) + __filling * (__length - __string_length)


def has_extension(__filename: str, __target_extension: str) -> bool: ...
def has_path_dir(__filename: str) -> bool: ...



@dataclass
class Registry(object):
    key: int
    data_1: int
    data_2: str
    data_3: str

    def struct_bytes(self) -> bytes:
        """Assembles the Registry information into a (C) struct, represented in bytes."""

        __key_data1: bytes = pack("il", self.key, self.data_1)
        __data2: bytes = pack(Constants.data_2_pack_fstr, fullfill_string_bytes(self.data_2, Constants.data_2_string_length))
        __data3: bytes = pack(Constants.data_3_pack_fstr, fullfill_string_bytes(self.data_3, Constants.data_3_string_length))
        
        return __key_data1 + __data2 + __data3


class DataFactory(object):
    """
        TODO: The data has to be generated based on something...
    """

    __acc_data: list[Registry]
    
    def __init__(self) -> None:
        self.__acc_data: list[Registry] = list()
    
    def write(self, __filename: str, sort: bool = True) -> bool:
        
        # Sorting by key
        if sort:
            self.__acc_data.sort(key=lambda r: r.key)


        with open(f"{__filename}", "wb") as bin_datafile:
            
            # For instance, let's just say that the header only contains
            # the number of elements and that is it...
            
            bin_datafile.write(pack("i", len(self.__acc_data)))
            
            # Then the data itself...

            for r in self.__acc_data:
                bin_datafile.write(r.struct_bytes())
            
        # raise NotImplementedError

    def add_data(self, __reg: Registry) -> None: ...
    def gen_random_data(self) -> Registry: ...
        
    @staticmethod
    def gen_random_noisy_registry() -> Registry:
        from random import choice, randint

        KEY_MIN: int = Constants.INT_MIN // 2 ** 8
        KEY_MAX: int = Constants.INT_MAX // 2 ** 8

        key: int = randint(KEY_MIN, KEY_MAX)
        data_1: int = randint(Constants.INT_MIN, Constants.INT_MAX)
               
        data_2: str = "".join([choice(alpha_numericals) for _ in range(Constants.data_2_string_length - 1)]) + "\0"
        data_3: str = "".join([choice(alpha_numericals) for _ in range(Constants.data_3_string_length - 1)]) + "\0"

        r: Registry = Registry(key, data_1, data_2, data_3)
        
        print(f"Generated: {r}")
        
        return r

    def gen_random_noisy_data(self, N: int) -> None:
        for _ in range(N):
            self.__acc_data.append(self.gen_random_noisy_registry())
        return None


if __name__ == "__main__":
    # r1: Registry = Registry(1, 123, "nothing", "very nothing")
    
    df: DataFactory = DataFactory()
    df.gen_random_noisy_data(13)
    df.write("nothing.bin")


    