#ifndef QOPENGLWIDGETCLUSTER_H
#define QOPENGLWIDGETCLUSTER_H

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QWidget>

#include "GeometryEngine.h"
#include <QBasicTimer>
#include <QQuaternion>

#include <QWheelEvent>
#include <QOpenGLFunctions_3_3_Core>
//#include <QOpenGLFunctions_4_5_Core>

class QOpenGLWidgetCluster : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    QOpenGLWidgetCluster(QWidget* parent = nullptr);
    ~QOpenGLWidgetCluster();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

//    void mousePressEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event) override;

    //    void timerEvent(QTimerEvent * e) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

private:
    void updateProjection();

private:
    //    QOpenGLVertexArrayObject m_vao;
    QOpenGLShaderProgram m_program[2];

    uint vbo[2];
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_vbo2;
    QOpenGLBuffer m_ebo;
    uint ebo;
    QOpenGLVertexArrayObject m_vao;

    uint vertexShader;
    uint fragmentShader;
    uint shaderProgram;

    uint vao[2];
//    uint vao2;

    float triangle [9] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    float m_vertices[36] {


        -0.5f, 0.5f, -5.0f,
        -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 2.0f,

        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 0.0f,

        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.0f, 0.0f, 0.0f,
    };

    float m_box[24] {
        -1, -1, -1,
        1, -1, -1,
        1, 1, -1,
        -1, 1, -1,

        -1, -1, 1,
        1, -1, 1,
        1, 1, 1,
        -1, 1, 1,
    };

    uint m_boxIndices[24] {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,

        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
//        4, 0, 3,
//        4, 3, 7,
//        3, 2, 6,
//        3, 6, 7,
//        0, 1, 2,
//        0, 2, 3,

//        1, 5, 6,
//        1, 6, 2,
//        4, 5, 1,
//        4, 1, 0,
//        7, 6, 5,
//        7, 5, 4
    };

    //    QBasicTimer m_timer;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;

    int m_projection_loc[2];
    int m_view_loc[2];
    int m_model_loc[2];
    float m_xRot =0;
    float m_yRot =0;
    float m_zRot =0;
    float m_fov = 75;

    float m_zCamera = -3;

    QPoint posFirstClicked;

    QVector2D m_mousePressPosition;
    qreal m_angularSpeed;
    QQuaternion m_rotation;

    bool mouseClicked = false;
};

#endif // QOPENGLWIDGETCLUSTER_H
