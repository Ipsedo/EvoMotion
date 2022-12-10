from typing import List, Tuple

from .shapes import Shape


class ObjShape(Shape):
    def __init__(self, obj_file_path: str):
        vertices_ref: List[Tuple[float, float, float]] = []
        normals_ref: List[Tuple[float, float, float]] = []

        vertices_draw_order: List[int] = []
        normals_draw_order: List[int] = []

        with open(obj_file_path, mode="r", encoding="utf-8") as obj_file:
            for line in obj_file:
                line_split = line.split(" ")
                match line_split[0]:
                    case "vn":
                        normals_ref.append(
                            (
                                float(line_split[1]),
                                float(line_split[2]),
                                float(line_split[3]),
                            )
                        )
                    case "v":
                        vertices_ref.append(
                            (
                                float(line_split[1]),
                                float(line_split[2]),
                                float(line_split[3]),
                            )
                        )
                    case "f":
                        vertices_draw_order.append(
                            int(line_split[1].split("/")[0])
                        )
                        vertices_draw_order.append(
                            int(line_split[2].split("/")[0])
                        )
                        vertices_draw_order.append(
                            int(line_split[3].split("/")[0])
                        )

                        normals_draw_order.append(
                            int(line_split[1].split("/")[2])
                        )
                        normals_draw_order.append(
                            int(line_split[2].split("/")[2])
                        )
                        normals_draw_order.append(
                            int(line_split[3].split("/")[2])
                        )

            self.__vertices: List[Tuple[float, float, float]] = []
            self.__normals: List[Tuple[float, float, float]] = []

            for v_idx, n_idx in zip(vertices_draw_order, normals_draw_order):
                self.__vertices.append(vertices_ref[v_idx - 1])
                self.__normals.append(normals_ref[n_idx - 1])

    def get_vertices(self) -> List[Tuple[float, float, float]]:
        return self.__vertices

    def get_normals(self) -> List[Tuple[float, float, float]]:
        return self.__normals
