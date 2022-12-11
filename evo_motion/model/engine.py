from typing import Dict

from glm import mat4

from .item import Item


class Engine:
    def __init__(self) -> None:
        self.__items: Dict[str, Item] = {}

    def step(self, delta: float) -> None:
        pass

    def add_item(self, item: Item) -> None:
        self.__items[item.name] = item

    def get_model_matrix(self, item_name: str) -> mat4:
        return self.__items[item_name].model_matrix()
