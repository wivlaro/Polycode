/*
Copyright (C) 2011 by Ivan Safrin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
 
#pragma once
#include "PolyGlobals.h"
#include "PolyVertex.h"

class OSFILE;

namespace Polycode {
	
	class String;

	class _PolyExport VertexSorter : public PolyBase {
		public:
			Vertex *target;
			bool operator() (Vertex *v1,Vertex *v2) { return (v1->distance(*target)<v2->distance(*target));}
	};	
	
	class _PolyExport VertexBuffer : public PolyBase {
		public:	
			VertexBuffer(){}
			virtual ~VertexBuffer(){}
		
			int getVertexCount() const { return vertexCount;}
		
			int verticesPerFace;
			int meshType;
		protected:
		int vertexCount;
			
	};
	
	/**
	* Render data array.
	*/
	class _PolyExport RenderDataArray : public PolyBase {
	public:		
		int arrayType;
		int stride;
		int size;
		void *arrayPtr;
		void *rendererData;
		int count;
		
		/**
		* Vertex position array.
		*/
		static const int VERTEX_DATA_ARRAY = 0;
		
		/**
		* Vertex color array.
		*/		
		static const int COLOR_DATA_ARRAY = 1;		
		
		/**
		* Vertex normal array.
		*/				
		static const int NORMAL_DATA_ARRAY = 2;				

		/**
		* Vertex texture coordinate array.
		*/						
		static const int TEXCOORD_DATA_ARRAY = 3;
		
		/**
		* Tangent vector array.
		*/				
		static const int TANGENT_DATA_ARRAY = 4;				
		
		
	};
		

	typedef struct {
		float x;
		float y;
		float z;
		float w;		
	} Vector4_struct;
	
	typedef struct {
		float x;
		float y;
		float z;
	} Vector3_struct;

	typedef struct {
		float x;
		float y;
	} Vector2_struct;
	
	/**
	*  A mesh comprised of vertices. When data in the mesh changes, arrayDirtyMap must be set to true for the appropriate array types (color, position, normal, etc). Available types are defined in RenderDataArray.
	*/
	class _PolyExport Mesh : public PolyBase {
		public:
		
			
			/**
			* Construct with an empty mesh of specified type.
			* @param meshType Type of mesh. Possible values are: Mesh::QUAD_MESH, Mesh::TRI_MESH, Mesh::TRIFAN_MESH, Mesh::TRISTRIP_MESH, Mesh::LINE_MESH, Mesh::POINT_MESH.
			*/			
			Mesh(int meshType);
					
			/**
			* Construct from a mesh loaded from a file.
			* @param fileName Path to mesh file.
			*/
			Mesh(const String& fileName);

			/**
			* Construct from a mesh loaded from a file.
			* @param fileName Path to mesh file.
			*/			
			static Mesh *MeshFromFileName(String& fileName);

			virtual ~Mesh();
			
			/**
			* Loads a mesh from a file.
			* @param fileName Path to mesh file.
			*/			
			void loadMesh(const String& fileName);
			
			/**
			* Clears mesh data.
			*/
			
			void clearMesh();

			/**
			* Saves mesh to a file.
			* @param fileName Path to file to save to.
			*/			
			void saveToFile(const String& fileName, bool writeNormals = true, bool writeTangents = true, bool writeColors = true, bool writeBoneWeights = true, bool writeUVs = true, bool writeSecondaryUVs = false);

			void loadFromFile(OSFILE *inFile);
			void saveToFile(OSFILE *outFile, bool writeNormals = true, bool writeTangents = true, bool writeColors = true, bool writeBoneWeights = true, bool writeUVs = true, bool writeSecondaryUVs = false);
			
			
			/**
			* Returns the total vertex count in the mesh.
			* @return Number of vertices in the mesh.
			*/
			unsigned int getVertexCount();
			
					
			/**
			* Creates a plane mesh of specified size.
			* @param w Width of plane.
			* @param h Depth of plane.			
			*/ 
			void createPlane(Number w, Number h);
			
			/**
			* Creates a vertical plane mesh of specified size.
			* @param w Width of plane.
			* @param h Depth of plane.			
			*/ 
			void createVPlane(Number w, Number h);

			/**
			* Creates a 2D circle.
			* @param w Width of circle.
			* @param h Height of plane.			
			* @param numSegments Number of segments 			
			*/ 
			void createCircle(Number w, Number h, unsigned int numSegments);

            /**
             * Creates a 2D circle with normals pointing outwards from vertices.
             * @param w Width of circle.
             * @param h Height of plane.
             * @param numSegments Number of segments
             */
            void createLineCircle(Number w, Number h, unsigned int numSegments);

			/**
			* Creates a torus.
			* @param radius Radius of the torus.
			* @param tubeRadius Radious of the tube.
			* @param rSegments Number of radial segments.
			* @param tSegments Number of tube segments.
			*/ 	
			void createTorus(Number radius, Number tubeRadius, int segmentsW, int segmentsH);
			
			/**
			* Creates a cube mesh of specified size.
			* @param w Width of cube.
			* @param d Depth of cube.			
			* @param h Height of cube.
			*/ 			
			void createBox(Number w, Number d, Number h);
			
			/**
			* Creates a sphere mesh of specified size.
			* @param radius Radius of sphere.
			* @param numRings Number of rings.	
			* @param numSegments Number of segments.
			*/ 						
			void createSphere(Number radius, int numRings, int numSegments);

			/**
			* Creates a cylinder mesh.
			* @param height Height of the cylinder.
			* @param radius Radius of the cylinder.
			* @param numSegments Number of segments.
			* @param capped Create the end caps.
			*/ 								
			void createCylinder(Number height, Number radius, int numSegments, bool capped=true);

			/**
			* Creates a cone mesh.
			* @param height Height of the cone.
			* @param radius Radius of the cone.
			* @param numSegments Number of segments.
			*/ 								
			void createCone(Number height, Number radius, int numSegments);

		
			/**
			* Recenters the mesh with all vertices being as equidistant from origin as possible.
			*/
			Vector3 recenterMesh();
		
            Vertex *addVertex(Number x, Number y, Number z, Number u, Number v);
        
            Vertex *addVertex(Number x, Number y, Number z);
			
            void addVertex(Vertex *vertex);
        
            Vertex *getVertex(unsigned int index) const;

            Vertex *getActualVertex(unsigned int index) const;

            unsigned int getActualVertexCount() const;

			/** Copies all the faces from the mesh given into this mesh. Meshes must be the same type.
				Optionally transforms the vertices as well. */
			void addMesh(Mesh* sourceMesh, Matrix4* sourceTransform = NULL);
        
        
			/**
			* Sets the vertex buffer for the mesh.
			* @param buffer New vertex buffer for mesh.
			*/			
			void setVertexBuffer(VertexBuffer *buffer);
			
			/**
			* Returns the vertex buffer for the mesh.
			* @return The vertex buffer for this mesh.
			*/
			VertexBuffer *getVertexBuffer();
        
        
            Mesh *Copy() const;
			
			/**
			* Returns the radius of the mesh (furthest vertex away from origin).
			* @return Mesh radius.
			*/			
			Number getRadius();
			
			/**
			* Recalculates the mesh normals
			* @param smooth If true, will use smooth normals.
			* @param smoothAngle If smooth, this parameter sets the angle tolerance for the approximation function.
			*/
			void calculateNormals(bool generateFaceNormals = false);

			/**
			* Recalculates the tangent space vector for all vertices.
			*/ 
			void calculateTangents();
			
			/**
			* Returns the mesh type.
			*/ 
			int getMeshType();
			
			/**
			* Sets a new mesh type.
			* @param newType New mesh type. Possible values are: Mesh::QUAD_MESH, Mesh::TRI_MESH, Mesh::TRIFAN_MESH, Mesh::TRISTRIP_MESH, Mesh::LINE_MESH, Mesh::POINT_MESH.
			*/ 
			void setMeshType(int newType);

			void dirtyArray(unsigned int arrayIndex);
			void dirtyArrays();

            void setUseFaceNormals(bool val);
            bool getUseFaceNormals();
        
			/**
			* Calculates the mesh bounding box.
			*/
			Vector3 calculateBBox();

			/**
			* Checks if the mesh has a vertex buffer.
			* @param True if the mesh has a vertex buffer, false if not.
			*/		
			bool hasVertexBuffer() { return meshHasVertexBuffer; }
	
			/**
			* Quad based mesh.
			*/
			static const int QUAD_MESH = 0;			
			
			/**
			* Triangle based mesh.
			*/			
			static const int TRI_MESH = 1;				
			
			/**
			* Triangle fan based mesh.
			*/						
			static const int TRIFAN_MESH = 2;

			/**
			* Line based mesh.
			*/									
			static const int LINE_MESH = 4;
			
			/**
			* Point based mesh.
			*/									
			static const int POINT_MESH = 5;
		
			/**
			* Line strip based mesh.
			*/									
			static const int LINE_STRIP_MESH = 6;
			
			/**
			* Line loop based mesh.
			*/									
			static const int LINE_LOOP_MESH = 7;
		
		
			/**
			* Render array dirty map. If any of these are flagged as dirty, the renderer will rebuild them from the mesh data. See RenderDataArray for types of render arrays.
			* @see RenderDataArray
			*/
			bool arrayDirtyMap[16];
			
			/**
			* Render arrays. See RenderDataArray for types of render arrays.
			* @see RenderDataArray			
			*/			
			RenderDataArray *renderDataArrays[16];
		
			/**
			* If set to true, the renderer will use the vertex colors instead of entity color transform to render this mesh.
			*/
			bool useVertexColors;
            bool indexedMesh;

            void addIndexedFace(unsigned int i1, unsigned int i2);
            void addIndexedFace(unsigned int i1, unsigned int i2, unsigned int i3);
            void addIndexedFace(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4);
            void addIndex(unsigned int index);
        
            Vector3 getFaceNormalForVertex(unsigned int index);
        
            void addFaceNormal(Vector3 faceNormal);
        
		protected:
        
            Vector3 calculateFaceTangent(Vertex *v1, Vertex *v2, Vertex *v3);
        
            bool useFaceNormals;
        
            VertexBuffer *vertexBuffer;
            bool meshHasVertexBuffer;
            int meshType;
        
            std::vector<Vector3> faceNormals;
            std::vector<unsigned int> indices;
            std::vector <Vertex*> vertices;
	};
}
