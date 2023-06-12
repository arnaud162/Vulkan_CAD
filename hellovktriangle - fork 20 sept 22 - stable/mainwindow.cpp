#ifndef __IOSTREAM_H
#include <iostream>
#endif
#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVulkanFunctions>
#include <QWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStandardItemModel>
#include <algorithm>
#include <QHeaderView>
#include <QObject>


using namespace std;


//std::vector<Vertex> Vertices ;

int numeroLigneSelectionnee=0;
QString parallelipedeOuCylindre ;
QStandardItemModel *model=new QStandardItemModel();


MainWindow::MainWindow(VulkanWindow *w) : m_window(w)
{
    pm_renderer=w->m_renderer;
    QWidget *wrapper = createWindowContainer(w);
    QHBoxLayout *layout = new QHBoxLayout;

    QFile file("voulu2.csv");
    if (file.open(QFile::ReadOnly)) {
        QTextStream stream(&file);

        QString line = stream.readLine();
        QStringList list = line.remove('"').simplified().split(',');
        model->setHorizontalHeaderLabels(list);

        int row = 0;
        QStandardItem *newItem = nullptr;
        while (!stream.atEnd()) {
            line = stream.readLine();
            if (!line.startsWith('#') && line.contains(',')) {
                list = line.remove('"').simplified().split(',');
                for (int col = 0; col < list.length(); ++col){
                    newItem = new QStandardItem(list.at(col));
                    model->setItem(row, col, newItem);
                }
                ++row;
            }
        }
    }
    file.close();

    m_table = new QTableView;
    m_table->setModel(model);
    m_table->horizontalHeader()->setMinimumSectionSize(1);
    //m_table->setMin
    m_table->resizeColumnsToContents();
    //m_table->setShowGrid(false);
//    m_table->setColumnWidth(0,50);
//    m_table->setColumnWidth(1,50);
    for (int i=3;i<15; i++){
        m_table->setColumnWidth(i,35   );
    }

    //récupère la ligne sélectionnée du tableau
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged,this,&MainWindow::clicked);
    // fait tourner le modèle vulkan


    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged, w, [=]{
        w->changeColor();
    });

    connect(m_table->model(), &QStandardItemModel::dataChanged, this,&MainWindow::debugChangementModele
    );
   layout->addWidget(m_table,4);
   layout->addWidget(wrapper,6);
   setLayout(layout);
}



QVulkanWindowRenderer *VulkanWindow::createRenderer()
{
    m_renderer = new TriangleRenderer(this, true );
    //Vertices = *m_renderer->pvertices;
    return m_renderer ; // try MSAA, when available*/

}

 std::vector <Vertex> v;

// void MainWindow::modelHasBeenChanged(){
//     qDebug()<<"émission du signal que le modèle a été changé";
// }
void MainWindow::clicked()
{
    qDebug() << "\nchangment select detecté : ";
    QItemSelectionModel *selectonModel = m_table ->selectionModel();
    qDebug() << selectonModel->selectedIndexes()[0].row()<<"\n";
    numeroLigneSelectionnee = selectonModel->selectedIndexes()[0].row();
    parallelipedeOuCylindre = m_table->model()->data( m_table->model()->index(numeroLigneSelectionnee,1)).toString();
    //qDebug() <<"contenu de la case sélectionnée"<<m_table->model()->data( m_table->model()->index(numeroLigneSelectionnee,1)).toString();
//std::cout << "taille de l'array : "<<Vertices.size() <<"\n";
//   QVector3D vvv = Vertices.data()[19900].color;
//    std :: cout << vvv[0] << " ";
//    std :: cout << vvv[1] << " ";
//    std :: cout << vvv[2] << "\n";

//    for ( int i=Vertices.size()-1000; i<Vertices.size(); i++){
//        Vertices.data()[i].color = {0.5f,1.0f,0.5f};
//    }

    //pm_renderer->rotate(20.0f);
}

void MainWindow::debugChangementModele(){ // trouver un moyen pour que vkrenderer recharge la modele
    qDebug() << "\nLe modèle a changé";
    //  reeecriture du fichier csv (1/2)
    QFile file("voulu2.csv");
    if(file.open(QIODevice::WriteOnly)){
         QDataStream stream(&file);
         int rowCount = model->rowCount();
         int columnCount = model->columnCount();
        //QString aEcrireDansLeFichier;
         for(int column = 0; column < columnCount; column++) { // ecriture du header
             QByteArray aEcrireDansLeFichier;
             aEcrireDansLeFichier+= model->horizontalHeaderItem(column)->text();
             //aEcrireDansLeFichier+= model->item(row, column)->text();
             file.write(aEcrireDansLeFichier); //conversion de qstring a qbytearray
             if (column!=columnCount-1) file.write( ","); //il faut mettre des , entre chaque elmt sauf à la fin de la ligne
         }
         file.write("\n");
         for(int row = 0; row < rowCount; row++){
                 for(int column = 0; column < columnCount; column++) {
                    file.write("\"");
                     QByteArray aEcrireDansLeFichier;
                     aEcrireDansLeFichier+= model->item(row, column)->text();
                     file.write(aEcrireDansLeFichier); //conversion de qstring a qbytearray
                     if (column==columnCount-1) file.write( "\"");//il faut mettre des , entre chaque elmt sauf à la fin de la ligne
                     else file.write( "\",");
                 }
                 file.write("\n");
         }
    }

    // changement de la visualisation (2/2)
    pm_renderer->reloadVkModel(); // le changement va s'effectuer dans la visualisation
}



//slots
void VulkanWindow::rotate(){
    m_renderer->rotate(120);
}

void VulkanWindow::changeColor()
{
    if (parallelipedeOuCylindre=="p" or parallelipedeOuCylindre=="c") m_renderer->changeColor(numeroLigneSelectionnee); // -1 sur numerodeligne pour cause de resolution de bug chelou
    else  m_renderer->changeColor(-1);
    qDebug() << "\nchangment select detecté : ";
//    QItemSelectionModel *selectonModel = m_table ->selectionModel();
    //    qDebug() << selectonModel->selectedIndexes()[0].row()<<"\n";
}




// contrôles / déplacements
void VulkanWindow::keyPressEvent(QKeyEvent *e)
{
        const float amount = e->modifiers().testFlag(Qt::ShiftModifier) ? 1.0f : 0.1f;
        switch (e->key()) {
        case Qt::Key_Z:
            m_renderer->walk(amount);
            break;
        case Qt::Key_S:
            m_renderer->walk(-amount);
            break;
        case Qt::Key_Q:
            m_renderer->strafe(-amount);
            break;
        case Qt::Key_D:
            m_renderer->strafe(amount);
            break;
        default:
            break;
        }
    }

void VulkanWindow::mousePressEvent(QMouseEvent *e)
{
    m_pressed = true;
    m_lastPos = e->pos();
    b= e->button();
}

void VulkanWindow::mouseReleaseEvent(QMouseEvent *e)
{
    m_pressed = false;
}

void VulkanWindow::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_pressed)
        return;

    int dx = e->pos().x() - m_lastPos.x();
    int dy = e->pos().y() - m_lastPos.y();

    if (dy)
        m_renderer->pitch(dy / 10.0f);

    if (dx)
        m_renderer->yaw(dx / 10.0f);
    m_lastPos = e->pos();
}

void VulkanWindow::wheelEvent(QWheelEvent *e){
    m_renderer->rotate(e->angleDelta().ry());
    std::cout<<e->angleDelta().ry();
}


