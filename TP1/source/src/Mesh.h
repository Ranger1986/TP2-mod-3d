#ifndef MESH_H
#define MESH_H

struct Mesh {
    std::vector <Vec3> vertices; //array of mesh vertices positions
    std::vector <Vec3> normals; //array of vertices normals useful for the display
    std::vector <Triangle> triangles; //array of mesh triangles
    std::vector <Vec3> triangle_normals; //triangle normals to display face normals

    //Compute face normals for the display

    void computeTrianglesNormals() {
        // Vider le vecteur triangle_normals (i.e. faire un clear du vecteur)
        triangle_normals.clear();
        Vec3 e_10;
        Vec3 e_20;
        //TODO: implémenter le calcul des normales par face
        //Iterer sur les triangles du maillage
        for (int i; i<triangles.size(); ++i){
            e_10 = vertices[triangles[i][1]]-vertices[triangles[i][0]];
            e_20 = vertices[triangles[i][2]]-vertices[triangles[i][0]];
            Vec3 n =Vec3::cross(e_10, e_20);
            n.normalize();
            triangle_normals.push_back(n);
        }

            //La normal du triangle i est le resultat du produit vectoriel de deux ses arêtes e_10 et e_20 normalisé (e_10^e_20)
            //L'arete e_10 est représentée par le vecteur partant du sommet 0 (triangles[i][0]) au sommet 1 (triangles[i][1])
            //L'arete e_20 est représentée par le vecteur partant du sommet 0 (triangles[i][0]) au sommet 2 (triangles[i][2])

            //Normaliser le resultat, utiliser la fonction normalize()

            //Ajouter dans triangle_normales

    }

    //Compute vertices normals as the average of its incident faces normals
    void computeVerticesNormals() {
        // Vider le vecteur normals (i.e. faire un clear du vecteur)
        normals.clear();
        std::vector<float> n;
        //TODO: implémenter le calcul des normales par sommet comme la moyenne des normales des triangles incidents
        //Initializer le vecteur normals taille vertices.size() avec Vec3(0., 0., 0.)
        for (int i; i<vertices.size(); ++i){
            normals.push_back(Vec3(0.0,0.0,0.0));
            n.push_back(0);
        }
        
        //Iterer sur les triangles
        for (int i; i<triangles.size(); ++i){
            normals[triangles[i][0]]+= triangle_normals[i];
            normals[triangles[i][1]]+= triangle_normals[i];
            normals[triangles[i][2]]+= triangle_normals[i];
            n[triangles[i][0]]+= 1;
            n[triangles[i][1]]+= 1;
            n[triangles[i][2]]+= 1;
        }

            //Pour chaque triangle i
            //Ajouter la normal au triangle à celle de chacun des sommets

        //Iterer sur les normales et les normaliser
        for (int i; i<vertices.size(); ++i){
            normals[i]=normals[i]/(float)n[i];
            normals[i].normalize();
        }
    }

    void computeNormals() {
        computeTrianglesNormals();
        computeVerticesNormals();
    }

    Mesh(){}

    Mesh( Mesh const& i_mesh):
        vertices(i_mesh.vertices),
        normals(i_mesh.normals),
        triangles(i_mesh.triangles),
        triangle_normals(i_mesh.triangle_normals)
    {}

    Mesh( std::vector <Vec3> const& i_vertices, std::vector <Triangle> const& i_triangles):
        vertices(i_vertices),
        triangles(i_triangles)
    {
        computeNormals();
    }

};


#endif // MESH_H
