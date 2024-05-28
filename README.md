# PhysXParser

Why not just use the PhysX SDK? Because..

* I didn't want to include the whole SDK just to read meshes.
* I need to convert these to 3D model vertices/indices anyway.
* I need to understand the format. This is a good way to learn.

# What's Supported?

Currently just Height Fields. But this repo exists in case more support is needed. Feel free to contribute or contact me about this.

Here's my notes about all the mesh formats. And for that juicy SEO in case someone searches the FourCC they just found.

| Name | FourCC | Latest | Parent | [SDK location](https://github.com/NVIDIA-Omniverse/PhysX/tree/main/physx) | 
| ---- | ------ | ------ | ------ | ------------ | 
| Adjacencies | ADJA | ? | | `source/geomutils/src/common/GuAdjacencies.cpp` (Commented out?) |
| Bounding Volume Hierarchy | BVHS | 1 | | `source/geomutils/src/GuBVH.*` |
| Convex Mesh | CVXM | 14 | | `source/geomutils/src/convex` |
| Convex Hull | CLHL, CVHL | 14 | ConvexMesh |`source/geomutils/src/convex` | 
| Gauss Map | GAUS | 9  | SUPM | Write: `source/geomutils/src/cooking/GuCookingBigConvexDataBuilder.cpp` Read: `source/geomutils/src/convex/GuBigConvexData.cpp` |
| Edge list | EDGE | ? | | `source/geomutils/src/common/GuEdgeList.cpp` |
| Height Field | HFHF | 3 | | `source/geomutils/src/hf` |
| Soft Body Mesh | SOME | 2 | | Write: `source/geomutils/src/cooking/GuCookingTetrahedronMesh.cpp` Read: `source/geomutils/src/GuMeshFactory.cpp` |
| Triangle Mesh | MESH | 16 | | Write: `source/geomutils/src/cooking/GuCookingTriangleMesh.cpp` Read: `source/geomutils/src/GuMeshFactory.cpp` |
| Tetrahedron Mesh | TEME | 1 | | Write: `source/geomutils/src/cooking/GuCookingTetrahedronMesh.cpp` Read: `source/geomutils/src/GuMeshFactory.cpp` |
| Valencies | VALE | 2 | GAUS | Write: `source/geomutils/src/cooking/GuCookingBigConvexDataBuilder.cpp` Read: `source/geomutils/src/convex/GuBigConvexData.cpp` |
| ? | SUPM | 0 | | Write: `source/geomutils/src/cooking/GuCookingBigConvexDataBuilder.cpp` Read: `source/geomutils/src/convex/GuBigConvexData.cpp` |