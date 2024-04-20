

from typing import Any, Callable


def MidpointRootedTraversal(__List: list[Any], __Action: Callable[[Any, Any, Any], None]) -> Any:
    def midpoint(l: int, r: int) -> int:
        return l + (r - l) // 2
    
    def traverse(l: int, r: int) -> Any | None:
        if l > r:
            return None
        
        m: int = midpoint(l, r)
        
        root: Any = __List[m]
        left: Any = traverse(l, m - 1)
        right: Any = traverse(m + 1, r)

        __Action(root, left, right)

        return root
    return traverse(0, len(__List) - 1)


def MRT_it(__List: list[Any], __Action: Callable[[Any, Any, Any], None]) -> Any:
    def midpoint(l: int, r: int) -> int:
        return l + (r - l) // 2

    __stack: list[(int, int)] = [(0, len(__List) - 1)]
    
    left: Any = None
    right: Any = None
    
    while bool(__stack):
        l, r = __stack.pop()
        m: int = midpoint(l, r)
        
        # print("-", __stack, f"-> {(l, r)}")
        left = right = None

        if l <= m - 1:
            __stack.append((l, m - 1))
            left = __List[midpoint(l, m - 1)]

        if m + 1 <= r:
            __stack.append((m + 1, r))
            right = __List[midpoint(m + 1, r)]

        root: Any = __List[m]
        print(root)
        __Action(root, left, right)
    return None


if __name__ == "__main__":
    A = ['a', 'b', 'c', 'd', 'e', 'f']

    def WorkOutNode(root, left, right):
        print(f"<{root}, {left}, {right}>")
        

    # print(MidpointRootedTraversal(A, WorkOutNode))
    print(MRT_it(A, WorkOutNode))

