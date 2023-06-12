// ce fichier a pour rôle de convertir les données du fichier csv en une struct de type vertex vertices
// + retourner indices
#ifndef __IOSTREAM_H
#include <iostream>
#endif

#ifndef _FSTREAM_
#include <fstream>
#endif

#include <string>
#include <sstream>
#include <QVulkanWindow>
#include <vector>
#include <QFile>
#include <QString>
#include <algorithm>


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "struct_vertex.h"

/// mainteant il faut que renderer acede au vecteur qui donne la corespndnace indice excel-vulkan


using namespace std;

struct Pair // changer le nom pour ternaire ou un truc comme ca
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vector <vector<int>> correspondanceVueExcelVueVulkan;
};


struct Pair lecture_csv(){
    int NOMBRE_SOMMETS_CYLINDRE=100;

    string line;
    vector <string> lines;


    //Chargement du repére depuis le fichier .obj
    std::vector<Vertex> verticesRepere;
    std::vector<uint32_t> indicesRepere;
    string MODEL_PATH = "repere.obj";
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            verticesRepere.push_back(vertex);
            indicesRepere.push_back(indicesRepere.size());
        }
    }


    // the grid. a digital frontier between z<0 and z>0.
    std::vector<Vertex> verticesOfTheGrid;
    std::vector <uint32_t> indicesOfTheGrid;
    QVector3D colorOfTheGrid = {0.f,0.f,1.0f};
    QVector2D textureOfTheGrid = {0.9f,0.9f};

    float sizeOfTheGrid = 20.0f; // describes extreme points of the grid
    float epaisseurTrait = 0.03f;
    for (float f=-sizeOfTheGrid; f<=sizeOfTheGrid; f++){
        verticesOfTheGrid.push_back({{f-epaisseurTrait,0.0f,-sizeOfTheGrid},colorOfTheGrid,textureOfTheGrid});
        verticesOfTheGrid.push_back({{f+epaisseurTrait,0.0f,-sizeOfTheGrid},colorOfTheGrid,textureOfTheGrid});


        verticesOfTheGrid.push_back({{-sizeOfTheGrid,0.0f,f-epaisseurTrait},colorOfTheGrid,textureOfTheGrid});
        verticesOfTheGrid.push_back({{-sizeOfTheGrid,0.0f,f+epaisseurTrait},colorOfTheGrid,textureOfTheGrid});


        verticesOfTheGrid.push_back({{f-epaisseurTrait,0.0f,sizeOfTheGrid},colorOfTheGrid,textureOfTheGrid});
        verticesOfTheGrid.push_back({{f+epaisseurTrait,0.0f,sizeOfTheGrid},colorOfTheGrid,textureOfTheGrid});


        verticesOfTheGrid.push_back({{sizeOfTheGrid,0.0f,f-epaisseurTrait},colorOfTheGrid,textureOfTheGrid});
        verticesOfTheGrid.push_back({{sizeOfTheGrid,0.0f,f+epaisseurTrait},colorOfTheGrid,textureOfTheGrid});
    }

    for (int i = 0; i<=sizeOfTheGrid*2*8;i+=8){ // *2 car on va de -sizeOfTheGrid à +sizeOfTheGrid; *8 car les vertices sont par blocs de 8
        indicesOfTheGrid.push_back(i);
        indicesOfTheGrid.push_back(i+1);
        indicesOfTheGrid.push_back(i+4);

        indicesOfTheGrid.push_back(i+1);
        indicesOfTheGrid.push_back(i+4);
        indicesOfTheGrid.push_back(i+5);


        indicesOfTheGrid.push_back(i+2);
        indicesOfTheGrid.push_back(i+3);
        indicesOfTheGrid.push_back(i+6);

        indicesOfTheGrid.push_back(i+2);
        indicesOfTheGrid.push_back(i+6);
        indicesOfTheGrid.push_back(i+7);
    }


/*
    std::vector<Vertex> verticesRepere = {
    {{0.0f,0.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{0.0f,0.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},

    {{0.0f,0.0f,10.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{-1.0f,-1.0f,10.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{-1.0f,-1.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},

    {{0.0f,10.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{-1.0f,10.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{-1.0f,0.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},

    {{10.0f,0.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{10.0f,-1.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{0.0f,-1.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},

    {{1.0f,1.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{9.0f,1.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{1.0f,9.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}},
    {{9.0f,9.0f,0.0f},{1.0f,0.0f,0.0f},{0.9f,0.9f}}
    };
    std::vector<uint32_t> indicesRepere = {
        1,2,3,
        1,3,4,

        1,5,6,
        1,6,7,

        1,8,9,
        1,9,10,

        11,12,13,
        12,13,14
    };
    */
    
    ifstream myfile ;
    myfile.open ("voulu2.csv");
    if (myfile.is_open()){
        while ( getline (myfile,line) ) {lines.push_back(line);}
        myfile.close();
    }
  
    else cout << "Unable to open file";
    myfile.close();
    vector <vector<string>> lines2;   // les mots sont séparés par ','
    string segment;
    std::vector<std::string> seglist;

    

    for (int i = 0; i < lines.size(); i++){
        std::stringstream test(lines[i]);
        seglist.clear();
        while(getline(test, segment, ',')){
            seglist.push_back(segment);
        }
        lines2.push_back(seglist);
        }
  //ici on remplace " par un espace. ca facilitera les conversions si il y a des virgules
  //std::string::size_type sz;
    for (int i = 0; i < lines2.size(); i++){
        for (int j = 0; j < lines2[i].size(); j++){
            std::replace(lines2[i][j].begin(), lines2[i][j].end(),'\"',' ');
        }
    }

    
    //int NOMBRE_SOMMETS_CYLINDRE=100;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

     // servira à faire en sorte que lorsqu'on clique sur un élément dans la vue
            // de type "excel" l'element correspondan change de couleur dans la vue vulkan
            // liste de couples d'indices : le premier pour le début des vertices, le deuxiéme pour la fin
    vector <vector<int>> indicesDesVertexDesElements;
    int indicePremierVertexDeLelementN ;
    int indiceDernierVertexDeLelementN ;


    std::vector<QVector3D> ListeDesSommets;
    QVector3D centre, sommet;
    QVector3D temporairec=QVector3D(0.0f, 1.0f, 0.0f);
    QVector2D textureCoord=QVector2D(0.1f, 0.1f);
    

    for (int i =0 ; i<verticesRepere.size();i++){
        vertices.push_back(verticesRepere[i]);
    }

    for (int i =0 ; i<indicesRepere.size();i++){
        indices.push_back(indicesRepere[i]);
    }


    int nombreDeVertexDuRepere = verticesRepere.size();

    int indiceSommetCourant =  indicesRepere.size();
    //int indiceSommetCourant =  0;

    QVector3D vec3; // le vecteur normal, obtenu par produit vectoriel
    std::vector<QVector3D> listeCoordonneesCGs;
    std::vector<Vertex> verticesCG ;
    std::vector<uint32_t> indicesCG;
    std::vector<uint32_t> listeDesPoids;
    int indiceSommetCourantCG;


    for (int i = 0; i < lines2.size(); i++){
        if (lines2[i][1]==" p ") {
            //code construction parallelepide
            ListeDesSommets.clear();
            centre = {stof(lines2[i][2]),stof(lines2[i][3]),stof(lines2[i][4])};
            
            //on constuit les 4 sommets du rectangle de base
            sommet={stof(lines2[i][6]),stof(lines2[i][7]),stof(lines2[i][8])};
            ListeDesSommets.push_back(sommet);
            
            sommet={stof(lines2[i][10]),stof(lines2[i][11]),stof(lines2[i][12])};
            ListeDesSommets.push_back(sommet);
            
            sommet={2*centre[0]-stof(lines2[i][6]),2*centre[1]-stof(lines2[i][7]),2*centre[2]-stof(lines2[i][8])};
            ListeDesSommets.push_back(sommet);

            sommet={2*centre[0]-stof(lines2[i][10]),2*centre[1]-stof(lines2[i][11]),2*centre[2]-stof(lines2[i][12])};
            ListeDesSommets.push_back(sommet);

            //on trouve le vecteur normal au plan
                //on commence par déterminer les coordonnées de deux vecteurs du rectangle de base
                QVector3D vec1 = centre-ListeDesSommets[0];
                QVector3D vec2 = centre-ListeDesSommets[1];
                vec3 = QVector3D::crossProduct(vec1,vec2); //maintenant qu'on les un simple cross product suffit
                vec3 = vec3.QVector3D::normalized() * stof (lines2[i][14]); // et on finit par la normalisation. ce vecteur est unitaire.

            

                //on ajoute les 4 sommets du rectangle opposé
            for (int k=0; k<4; k++) ListeDesSommets.push_back(ListeDesSommets[k]+vec3);

            indicePremierVertexDeLelementN = vertices.size();

            //on ajoute les sommets à la liste des vertices
            for (int k=0; k<8; k++) vertices.push_back({ListeDesSommets[k],temporairec, {0.5f, 0.5f}});


            // on va maintenant s'occuper du vecteur n*2 qui donne la correspondance vue excel vue vulkan, pour
            //changer la couleur des elements qaund ils sont clickés
            indiceDernierVertexDeLelementN = vertices.size()-1;
            indicesDesVertexDesElements.push_back({indicePremierVertexDeLelementN,indiceDernierVertexDeLelementN});



            

            // on va maintenant créer la liste des indices
           int j = indiceSommetCourant;
           std::vector<int> indicestemp={j,j+1,j+3,
           j, j+1, j+4,
           j, j+3, j+4,

           j+2, j+1, j+3,
           j+2, j+3, j+6,
           j+2, j+1, j+6,

           j+5, j+1,j+4,
           j+5, j+1, j+6,
           j+5, j+4, j+6,

           j+7, j+6, j+3,
           j+7, j+6, j+4,
           j+7, j+3, j+4
           };
           for (int j=0; j<indicestemp.size(); j++){
               indices.push_back(indicestemp[j]);
           }
            indiceSommetCourant=indiceSommetCourant+8;
            //ajout des coordonnées du CG
            listeCoordonneesCGs.push_back(centre+vec3*0.5f);
            listeDesPoids.push_back(stof(lines2[i][16]));
        }

        else if (lines2[i][1]==" c "){
            //code construction cylindre
            QVector3D centre  = {stof(lines2[i][2]),stof(lines2[i][3]),stof(lines2[i][4])};
            QVector3D vec1 ={centre[0]-stof(lines2[i][6]),centre[1]-stof(lines2[i][7]),centre[2]-stof(lines2[i][8])};
            QVector3D vec2 ={centre[0]-stof(lines2[i][10]),centre[1]-stof(lines2[i][11]),centre[2]-stof(lines2[i][12])};
            float r = max(vec1.QVector3D::length(),vec2.QVector3D::length());
            vec3 = QVector3D::crossProduct(vec1,vec2).QVector3D::normalized()* stof (lines2[i][14]);

            //technique du rectangle inscrit
            QVector3D  pt1  ={stof(lines2[i][6]),stof(lines2[i][7]),stof(lines2[i][8])};
            QVector3D  pt2 = {stof(lines2[i][10]),stof(lines2[i][11]),stof(lines2[i][12])};
            QVector3D  pt3  = (2.0f*centre)-pt1;
            QVector3D  pt4 = (2.0f*centre)-pt2;
            

            float increment =int((NOMBRE_SOMMETS_CYLINDRE/4))-1;

            QVector3D veccyl1 = (pt1-pt2)*(1/increment);
            QVector3D veccyl2  = (pt2-pt3)*(1/increment);
            QVector3D veccyl3  = (pt3-pt4)*(1/increment);
            QVector3D veccyl4  = (pt4-pt1)*(1/increment);

            
            std::vector<QVector3D> listeVecteursRectangleInscrit;

            listeVecteursRectangleInscrit.push_back(pt1);
            for(int j=0; j<increment; j++){
               listeVecteursRectangleInscrit.push_back(listeVecteursRectangleInscrit.back()-veccyl1);
            }

            listeVecteursRectangleInscrit.push_back(pt2);
            for(int j=0; j<increment; j++){
                listeVecteursRectangleInscrit.push_back(listeVecteursRectangleInscrit.back()-veccyl2);
            }

            listeVecteursRectangleInscrit.push_back(pt3);
            for(int j=0; j<increment; j++){
                listeVecteursRectangleInscrit.push_back(listeVecteursRectangleInscrit.back()-veccyl3);
            }

            listeVecteursRectangleInscrit.push_back(pt4);
            for(int j=0; j<increment; j++){
                listeVecteursRectangleInscrit.push_back(listeVecteursRectangleInscrit.back()-veccyl4);
            }

            ListeDesSommets.clear();

            // Ecriture du centre
            ListeDesSommets.push_back(centre);
            
            for (int j=0; j<listeVecteursRectangleInscrit.size(); j++){
                QVector3D vec = centre-listeVecteursRectangleInscrit[j];
                vec=vec.QVector3D::normalized()*r; //on est sur d'avoir le rayon
                ListeDesSommets.push_back(centre + vec);
            }

            // projection de l'autre face du disque
            int l = ListeDesSommets.size();
            for (int j=0; j<l; j++){
                //std::cout << l;
                ListeDesSommets.push_back(ListeDesSommets[j]+vec3);//* stof (lines2[i][14]));
            }
            //ajout du centre translaté à la fin. sera utile pour dessiner la deuxième face du cylindre
            ListeDesSommets.push_back(centre+vec3);//* stof (lines2[i][14]));

            indicePremierVertexDeLelementN = vertices.size();

            for (int j=0; j<ListeDesSommets.size(); j++){
                vertices.push_back({ListeDesSommets[j], temporairec, textureCoord});
            }

            // on va maintenant s'occuper du vecteur n*2 qui donne la correspondance vue excel vue vulkan, pour
            //changer la couleur des elements qaund ils sont clickés
            indiceDernierVertexDeLelementN = vertices.size(); // il semblerait qu'il y ait un bug ici et que le dernier sommet ne soit qu'un plus 1
            indicesDesVertexDesElements.push_back({indicePremierVertexDeLelementN,indiceDernierVertexDeLelementN});

            // on va maintenant créer la liste des indices
            std::vector<uint32_t> indicestemp ;
            indicestemp.clear();
            int indiceSommetCourantc=indiceSommetCourant;
            for (int j=2+indiceSommetCourantc; j<NOMBRE_SOMMETS_CYLINDRE+indiceSommetCourantc; j++){
                //triangle côté 1
                indicestemp.push_back(j);
                indicestemp.push_back(j+1);
                indicestemp.push_back(j+NOMBRE_SOMMETS_CYLINDRE);

                //triangle côté 2
                indicestemp.push_back(j+1);
                indicestemp.push_back(j+NOMBRE_SOMMETS_CYLINDRE);
                indicestemp.push_back(j+NOMBRE_SOMMETS_CYLINDRE+1);

                //remplissage de la face
                indicestemp.push_back(indiceSommetCourantc);
                indicestemp.push_back(j);
                indicestemp.push_back(j+1);

                //remplissage face translatée
                indicestemp.push_back(indiceSommetCourantc+1+NOMBRE_SOMMETS_CYLINDRE);
                indicestemp.push_back(j+NOMBRE_SOMMETS_CYLINDRE);
                indicestemp.push_back(j+1+NOMBRE_SOMMETS_CYLINDRE);


                indiceSommetCourant=indiceSommetCourant+2;
            }
            indiceSommetCourant=indiceSommetCourant+7;

            for (int j=0; j<indicestemp.size(); j++){
               indices.push_back(indicestemp[j]);
           }

           //std::cout << vertices, indices;
        //ajout des coordonnées du CG
        listeCoordonneesCGs.push_back(centre+vec3*0.5f);
        listeDesPoids.push_back(stof(lines2[i][16]));
        }

        else indicesDesVertexDesElements.push_back({-1,-1}); //-1 -1 pour indiquer qu'il s'agit d'une ligne non valide à ne pas prendre en compte

    }


    // roation de -90° des éléments autour de x
    QMatrix4x4 m;
    m.rotate(-90, 1.0,0 ,0); // creeation d'une matrice de roation de -90° autour de x. pour corriger les pb de syst de coordonnées differents

    for (int i=verticesRepere.size(); i< vertices.size(); i++)    { // on commence à verticeReperes pour ne pas tourner le repére
        vertices[i].pos=m*vertices[i].pos;
    }
    // fin du code pour la rotation de -90°

    for (int i=0; i<listeCoordonneesCGs.size(); i++){
        float x=listeCoordonneesCGs[i][0],y=listeCoordonneesCGs[i][1],z=listeCoordonneesCGs[i][2];
        verticesCG.push_back({{x,y,z},{0.0f,0.0f,1.0f},{0.8f,0.8f}});
        verticesCG.push_back({{x,y-.1f,z-.1f},{0.0f,0.0f,1.0f},{0.8f,0.8f}});
        verticesCG.push_back({{x,y+.1f,z-.1f},{0.0f,0.0f,1.0f},{0.8f,0.8f}});

        indicesCG.push_back(indiceSommetCourantCG);
        indicesCG.push_back(indiceSommetCourantCG+1);
        indicesCG.push_back(indiceSommetCourantCG+2);
        indiceSommetCourantCG=indiceSommetCourantCG+3;
    }

    //plus de 2000 lignes de code, et sans celles qui suivent elles sont toutes inutiles.
    float x=0;
    float y=0;
    float z=0;
    float poidsTotal=0;
    for(int i=0; i<listeCoordonneesCGs.size();i++){
        x=x+listeCoordonneesCGs[i][0]*listeDesPoids[i];
        y=y+listeCoordonneesCGs[i][1]*listeDesPoids[i];
        z=z+listeCoordonneesCGs[i][2]*listeDesPoids[i];
        poidsTotal=poidsTotal+listeDesPoids[i];
    }
    x=x/poidsTotal; y=y/poidsTotal; z=z/poidsTotal;
    std::cout << "\n"<<poidsTotal<<" "<<x<<" "<<y<<" "<<z<<"\n";
    // et enfin l'injection des vertices du CG
    verticesCG.push_back({{x,y,z},{1.0f,0.0f,1.0f},{0.8f,0.2f}});
    verticesCG.push_back({{x,y-.5f,z-.5f},{1.0f,0.0f,1.0f},{0.8f,0.2f}});
    verticesCG.push_back({{x,y+.5f,z-.5f},{1.0f,0.0f,1.0f},{0.8f,0.2f}});

    indicesCG.push_back(indiceSommetCourantCG);
    indicesCG.push_back(indiceSommetCourantCG+1);
    indicesCG.push_back(indiceSommetCourantCG+2);
    indiceSommetCourantCG=indiceSommetCourantCG+3;
    
    std::vector<Vertex> verticesTemp = vertices ;
    std::vector<uint32_t> indicesTemp = indices;
    //for (int i=0; i<indicesTemp.size(); i++) indicesTemp[i]=indicesTemp[i]+indiceSommetCourantCG; // pour prendre en compte les indices du CG ?

    vertices.clear();
    indices.clear();

    //for (int i=0; i<verticesCG.size(); i++) vertices.push_back(verticesCG[i]);
    //for (int i=0; i<indicesCG.size(); i++) indices.push_back(indicesCG[i]);

    for (int i=0; i<verticesTemp.size(); i++) vertices.push_back(verticesTemp[i]);
    for (int i=0; i<indicesTemp.size(); i++) indices.push_back(indicesTemp[i]);

    
    // pour le fichier de debug. réactiver le code si nécessaire
    /*ofstream myfile2;
    myfile2.open("voulu_c++.txt");
    for (int j=0; j<19; j++) myfile2 <<"\n";
    for (int j=0; j<vertices.size();j++){
    myfile2 <<std::to_string(vertices[j].pos[0])+" "+std::to_string(vertices[j].pos[1])+" "+std::to_string(vertices[j].pos[2]);
    myfile2 << "\n";}
    myfile2.close();*/


    for (int i=0; i<indicesOfTheGrid.size(); i++) indices.push_back(vertices.size()+indicesOfTheGrid[i]);
    for (int i=0; i<verticesOfTheGrid.size(); i++) vertices.push_back(verticesOfTheGrid[i]);
    return {vertices, indices, indicesDesVertexDesElements};
}

