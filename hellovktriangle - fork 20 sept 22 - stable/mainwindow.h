#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "trianglerenderer.h"

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVulkanWindow>
#include <QTableView>



class VulkanWindow : public QVulkanWindow
{

public:
    QVulkanWindowRenderer *createRenderer() override;
    TriangleRenderer *m_renderer;

public slots :
    void rotate();
    void changeColor();
    //void reloadVkModel(); // de type char* et non void à cause d'un bug incompéhensible dans connect
private:
    void keyPressEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *event) override;

    bool m_pressed = false;
    std::string b = "";
    QPoint m_lastPos;
};


class MainWindow : public QWidget
{

public:
    explicit MainWindow(VulkanWindow *w);
    VulkanWindow *m_window;
    TriangleRenderer *pm_renderer;



public slots :
    void clicked();
    void debugChangementModele();

private:


    QTableView *m_table;
};


#endif // MAINWINDOW_H
