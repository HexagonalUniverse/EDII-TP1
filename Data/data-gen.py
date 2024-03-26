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


@dataclass(frozen=True)
class Constants(object):
    """
        Where the script constants are all stored.
    """

    def __init__(self) -> None:
        # This object shall not be instantiated. While it does not happens of me finding a better 
        # method of imposing that restriction, an RuntimeError is raised at __init__.
        raise RuntimeError

    data_2_string_length: int = 1000
    data_3_string_length: int = 5000

    data_2_pack_fstr: str = f"{data_2_string_length}s"
    data_3_pack_fstr: str = f"{data_3_string_length}s"



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



class DataGen(object):
    """
        TODO: The data has to be generated based on something...
    """

    __acc_data: list[Registry]
    
    def __init__(self) -> None:
        raise NotImplementedError
    
    
    def write_data(self, __filename: str) -> bool:
        
        with open(f"{__filename}", "wb") as bin_datafile:
            ...
            
        raise NotImplementedError


    def add_data(self, __reg: Registry) -> None: ...
    def gen_random_data(self) -> Registry: ...


if __name__ == "__main__":
    r1: Registry = Registry(1, 123, "nothing", "very nothing")
    print(
        len(r1.struct_bytes())
    )
    