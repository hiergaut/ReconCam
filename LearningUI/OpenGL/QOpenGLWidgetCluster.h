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

#define BUFF_MAX 7000

class QOpenGLWidgetCluster : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    QOpenGLWidgetCluster(QWidget* parent = nullptr);
    ~QOpenGLWidgetCluster();

//    void setPoints(const std::vector<float> points, int nbDots, int nbBoxes);
    void render(const std::vector<float> points);
    void setArea(std::vector<float> vbo, std::vector<uint> ebo);

    void setNormalize(const QMatrix4x4 &normalize);

    void setZCamera(float zCamera);

    void setObject(const QMatrix4x4 &object);

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

    uint m_vbo[2];
    float m_vbo_data[2][BUFF_MAX];
    int m_vbo_dataLen[2] = {0};
//    QOpenGLBuffer m_vbo;
//    QOpenGLBuffer m_vbo2;
//    QOpenGLBuffer m_ebo;
    uint m_ebo[2];
    int m_ebo_data[2][BUFF_MAX];
    int m_ebo_dataLen[2] = {0};

//    QOpenGLVertexArrayObject m_vao;

//    uint vertexShader;
//    uint fragmentShader;
//    uint shaderProgram;

    uint m_vao[2];
//    uint vao2;

    float triangle [21] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f,
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f
    };

//    float m_dots[3 * 1024];
    float box [56] {
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

//    std::vector<float> m_dots;
//    std::vector<float> m_boxes;

//    int m_nbBoxes = 0;
//    int m_nbDots = 0;
//    QVector<float> m_dots;
//    uint m_nbDots;
//    size_t m_dotsSize;


    float m_vertices[84] {

        -0.5f, 0.5f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.8f,
        0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.8f,
        0.5f, 0.5f, 2.0f, 0.0f, 1.0f, 0.0f, 0.8f,

        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f,
        0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f,
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f,

        0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f,
        -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f,
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f
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

    float m_square[12] {
        -1, -1, 0,
        1, -1, 0,
        1, 1, 0,
        -1, 1, 0
    };

    uint m_squareEbo[8] {
        0, 1,
        1, 2,
        2, 3,
        3, 0
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

//    float ** m_area;

    //    QBasicTimer m_timer;

    QMatrix4x4 m_normalize;

    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    QMatrix4x4 m_object;

    int m_projection_loc[2];
    int m_view_loc[2];
    int m_model_loc[2];
    int m_normalize_loc[2];
    int m_object_loc[2];
    float m_xRot =0;
    float m_yRot =0;
    float m_zRot =0;
    float m_fov = 75;

    float m_zCamera = -2.5;

    QPoint posFirstClicked;

    QVector2D m_mousePressPosition;
    qreal m_angularSpeed;
    QQuaternion m_rotation;

    bool mouseClicked = false;
};

#endif // QOPENGLWIDGETCLUSTER_H
