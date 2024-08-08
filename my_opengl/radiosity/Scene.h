#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <set>
#include "vec3.h"
#include "uvec3.h"

#define HEMICUBE_RESOLUTION 128
#define MAX_FACES 4096

class Scene
{
    public:
        Scene() { }

        /** Load the scene from an '.obj' file (all faces should be triangles) */
        void loadFromOBJFile (const char* filename);

        /** Calculates the form factors for all face-pairs */
        void calculateFormFactors ();
        void calculateFormFactors_one (int f); // added by T.Narumi

        /** Runs one iteration of evaluating the radiosity equations for each face */
        void calculateRadiosities ();

        // added by T.Narumi
        void calculateGatherRadiosities ();
        void calculateAreas();
        void initializeUnshotRadiosities();
        void calculateProgressiveRadiosities ();

        /** Render the scene using colors interpolated across faces */
        void render();

        /** Render the scene with a unique color code for each face */
        void renderColorCoded();

        /** Calculate the normals for each face */
        void autoCalculateNormals();

        /** Populate the vector 'VtoF' with the neighbouring face data for each vertex */
        void buildVertexToFaceMap ();

        /** Interpolates the vertex colors based on surrounding face radiosities */
        void interpolateColors ();

        /** Get the color encoding for a given face */
        uvec3 encodeColor (unsigned int face);

        /** Get the face index from a given color encoding */
        unsigned int decodeColor (uvec3 color);

        // added by T.Narumi
        void set_output_fname(const char *fname_obj, const char *fname_mtl, float scale);
        void set_command_iterations(char com, int ite);
        void output_obj(void);

    protected:
        /** Vertices, faces and normals */
        std::vector<vec3> vertices;
        std::vector<uvec3> faces;
        std::vector<float> areas;  // added by T.Narumi
        std::vector<vec3> faceNormals;

        /** A map of each vertex's neighbouring faces */
        std::vector< std::vector<int> > VtoF;

        /** The list of emission factors for each face */
        std::vector<vec3> faceEmissions;

        /** The list of (color) reflectivities for each face */
        std::vector<vec3> faceReflectivities;

        /** An array of unique color codes for each face, used when rasterizing
         *  onto the hemicube */
        std::vector<uvec3> faceColorCodes;

        /** A 2-D array of formfactors "F" for the faces */
        std::vector< std::vector<float> > formFactors;

        /** The radiosities "H" for each face */
        std::vector<vec3> radiosities;
        std::vector<vec3> unshotradiosities; // added by T.Narumi

        /** Vertex colors after radiosity computation and interpolation across faces */
        std::vector<vec3> vertexColors;
};

#endif // SCENE_H
