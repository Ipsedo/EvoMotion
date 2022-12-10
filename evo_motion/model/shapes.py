from abc import ABC, abstractmethod
from typing import List, Tuple


class Shape(ABC):
    @abstractmethod
    def get_vertices(self) -> List[Tuple[float, float, float]]:
        pass

    @abstractmethod
    def get_normals(self) -> List[Tuple[float, float, float]]:
        pass
