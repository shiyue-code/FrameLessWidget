#ifndef FLWIDGET_H
#define FLWIDGET_H

#include <QMainWindow>

#include "fliocnfonthelp.h"

class QActionGroup;
class QToolButton;
class QLabel;

class FLHandle;

class FLTitle : public QWidget
{
    Q_OBJECT
public:
    enum TitleAction{
        TAIcon,
        TAMaxNormal,
        TAMin,
        TAFull,
        TAClose,
    };

public:
    FLTitle(QWidget *parent = nullptr);

    void defaultStyle();
    QList<QWidget *> ignoreWidgetList();

signals:
    void clk_close();
    void clk_maxnormal();
    void clk_min();
    void clk_full();
    void clk_icon();

public slots:
    void on_actions(TitleAction ta);

    void setIcon(const QString &str);
    void setIcon(const QIcon &icon);
    void setTitle(const QString &title);

private:
    void initActions();
    void initToolBtns();

private:
//    QWidget *titlebar;
    QLabel *title;
//    QWidget *tools;
    QToolButton *icon, *minBtn, *maxBtn, *closeBtn, *fullBtn;
    std::map<int, QAction*> m_actions;
    int titlebarHeight = 35;
};


class FLWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FLWidget(QWidget *parent = nullptr);

public:

    //设置是否可以通过鼠标调整窗口大小
    void setResizeable(bool resizeable=true);
    bool isResizeable();

    //设置可调整大小区域的宽度，在此区域内，可以使用鼠标调整窗口大小
    void setResizeableAreaWidth(int width = 5);

public:
    void setTitle(FLTitle *title, int titleHeight = -1);

    //FLWidget Contents Margins
    void setContentsMargins(int left, int top, int right, int bottom);
    void setContentsMargins(const QMargins &margins);
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    QMargins contentsMargins() const;
    QRect contentsRect() const;

public slots:
    void showFullScreen();
    void restoreScreen();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    FLHandle *m_flHandle;
    bool m_isMaxScreen;
};


class FLMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit FLMainWindow(QWidget *parent = nullptr);

public:

    //设置是否可以通过鼠标调整窗口大小
    void setResizeable(bool resizeable=true);
    bool isResizeable();

    //设置可调整大小区域的宽度，在此区域内，可以使用鼠标调整窗口大小
    void setResizeableAreaWidth(int width = 5);

public:
    void setTitle(FLTitle *title, int titleHeight = -1);

    //FLWidget Contents Margins
    void setContentsMargins(int left, int top, int right, int bottom);
    void setContentsMargins(const QMargins &margins);
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
    QMargins contentsMargins() const;
    QRect contentsRect() const;

public slots:
    void showFullScreen();
    void restoreScreen();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

private:
    FLHandle *m_flHandle;
    bool m_isMaxScreen;
};




#endif // FLWIDGET_H
