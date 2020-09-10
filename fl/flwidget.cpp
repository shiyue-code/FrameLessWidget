/*************** c++ **************/

/*************** qt **************/
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QLayout>
#include <QToolButton>
#include <QLabel>

#include <QDebug>

/*************** win *************/
#ifdef Q_OS_WIN

#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
#include <gdiplus.h>
#include <GdiPlusColor.h>
#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea
#pragma comment (lib,"user32.lib")

/*************** my **************/
#include "flwidget.h"


class FLHandle : public QObject
{
    Q_OBJECT
public:
    FLHandle(QWidget *w) :
        m_widget(w) ,
        m_title(nullptr) ,
        m_bResizeable(true) ,
        m_titleHeight(0) ,
        m_borderWidth(4) ,
        m_bJustMaximized(false)
    {
        assert(w);
    }

public:
    void init()
    {
        m_widget->installEventFilter(this);
        m_widget->setWindowFlags(m_widget->windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
        setResizeable(true);
        setContentsMargins(m_margins);
    }

    //设置m_widget的标题栏
    void setTitle(FLTitle *t, int titleHeight)
    {
        if(titleHeight >= 0)
            m_titleHeight = titleHeight;
        if(m_title)
        {
            delete m_title;
        }
        m_title = t;

        if(m_title)
        {
            m_title->setParent(m_widget);
            m_title->move(m_frames.left(), m_frames.top());
            m_title->resize(m_widget->width()-2*m_borderWidth, m_titleHeight);
            setContentsMargins(m_margins);

            for(QWidget *w: m_title->ignoreWidgetList())
            {
                addIgnoreWidget(w);
            }
            connect(m_title, SIGNAL(destroyed(QObject*)), this, SLOT(onTitleBarDestroyed()));

            connect(m_title, SIGNAL(clk_maxnormal()), this, SLOT(onMaxNormal()));
            connect(m_title, SIGNAL(clk_min()), this, SLOT(onMin()));
            connect(m_title, SIGNAL(clk_full()), this, SLOT(onFull()));
            connect(m_title, SIGNAL(clk_close()), this, SLOT(onClose()));
            connect(m_widget, SIGNAL(windowTitleChanged(const QString &)),
                    m_title, SLOT(setTitle(const QString&)));
            connect(m_widget, SIGNAL(windowIconChanged(const QIcon &)),
                    m_title, SLOT(setIcon(const QIcon &)));
            connect(m_widget, SIGNAL(windowIconTextChanged(const QString &)),
                    m_title, SLOT(setIcon(const QString &)));

        }
        else
        {
            setContentsMargins(m_margins);
        }
    }

    //设置是否可以通过鼠标调整窗口大小
    void setResizeable(bool resizeable=true)
    {
        HWND hwnd = (HWND)m_widget->winId();
        bool visible = m_widget->isVisible();
        auto winFlags = m_widget->windowFlags();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);

        m_bResizeable = resizeable;
        if (m_bResizeable)
        {

            winFlags |= Qt::WindowMaximizeButtonHint;
            //此行代码可以带回Aero效果，同时也带回了标题栏和边框,稍后在nativeEvent()会再次去掉标题栏
            style = style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION;
        }
        else
        {
            winFlags &= ~Qt::WindowMaximizeButtonHint;
            style = style & ~WS_MAXIMIZEBOX & ~WS_CAPTION;
        }

        m_widget->setWindowFlags(winFlags);
        ::SetWindowLong(hwnd, GWL_STYLE, style );
        //保留一个像素的边框宽度，否则系统不会绘制边框阴影
        const MARGINS shadow = { 1, 1, 1, 1 };
        DwmExtendFrameIntoClientArea(hwnd, &shadow);

        m_widget->setVisible(visible);
    }

    bool isResizeable()   { return m_bResizeable; }

    //设置可调整大小区域的宽度，在此区域内，可以使用鼠标调整窗口大小
    void setResizeableAreaWidth(int width = 5)
    {
        if (1 > width) width = 1;
        m_borderWidth = width;
    }

    void resizeEvent(QResizeEvent *event)
    {
        if(!m_title)
            return;
        m_title->move(m_frames.left(), m_frames.top());
        setContentsMargins(m_margins);
        m_title->resize(event->size().width()-m_frames.left()-m_frames.right(), m_titleHeight);
    }

    void setContentsMargins(int left, int top, int right, int bottom)
    {
        m_widget->QWidget::setContentsMargins(left+m_frames.left(),\
                                    top+m_frames.top() + m_titleHeight, \
                                    right+m_frames.right(), \
                                    bottom+m_frames.bottom());

        m_margins.setLeft(left);
        m_margins.setTop(top);
        m_margins.setRight(right);
        m_margins.setBottom(bottom);
    }

    void setContentsMargins(const QMargins &margins)
    {
        m_margins = margins;
        QMargins m = margins + m_frames;
        m.setTop(m.top() + m_titleHeight);
        m_widget->QWidget::setContentsMargins(m);
    }

    void getContentsMargins(int *left, int *top, int *right, int *bottom) const
    {
        if (!(left&&top&&right&&bottom)) return;

        auto  m = m_widget->QWidget::contentsMargins();
        *left = m.left();
        *top = m.top();
        *right = m.right();
        *bottom = m.bottom();

        if (m_widget->isMaximized())
        {
            *left -= m_frames.left();
            *top -= m_frames.top();
            *right -= m_frames.right();
            *bottom -= m_frames.bottom();
        }
        *top -= m_titleHeight;
    }

    QMargins contentsMargins() const
    {
        QMargins retm = m_widget->QWidget::contentsMargins();
        retm.setTop(retm.top() - m_titleHeight);
        retm -= m_frames;
        return retm;
    }

    QRect contentsRect() const
    {
        QRect rect = m_widget->QWidget::contentsRect();
        int width = rect.width();
        int height = rect.height();
        rect.setLeft(rect.left() - m_frames.left());
        rect.setTop(rect.top() - m_frames.top());
        rect.setWidth(width);
        rect.setHeight(height);
        return rect;
    }

    //在标题栏控件内，也可以有子控件如标签控件“label1”，此label1遮盖了标题栏，导致不能通过label1拖动窗口
    //要解决此问题，使用addIgnoreWidget(label1)
    void addIgnoreWidget(QWidget* widget)
    {
        if (!widget) return;
        if (m_whiteList.contains(widget)) return;
        m_whiteList.append(widget);
    }

    bool handleMessage(void *message, long *result, bool *isProcessed)
    {
        //Workaround for known bug -> check Qt forum : https://forum.qt.io/topic/93141/qtablewidget-itemselectionchanged/13
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
        MSG* msg = *reinterpret_cast<MSG**>(message);
#else
        MSG* msg = reinterpret_cast<MSG*>(message);
#endif

        switch (msg->message)
        {
        case WM_NCCALCSIZE:
        {
            *isProcessed = true;
            NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
            if (params.rgrc[0].top != 0)
                params.rgrc[0].top -= 1;

            //this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
            *result = WVR_REDRAW;
            return  true;
        }
        case WM_NCHITTEST:
        {
            *isProcessed = true;
            *result = 0;

            const LONG border_width = m_borderWidth;
            RECT winrect;
            GetWindowRect(HWND(m_widget->winId()), &winrect);

            long x = GET_X_LPARAM(msg->lParam);
            long y = GET_Y_LPARAM(msg->lParam);

            if(m_bResizeable)
            {

                bool resizeWidth = m_widget->minimumWidth() != m_widget->maximumWidth();
                bool resizeHeight = m_widget->minimumHeight() != m_widget->maximumHeight();

                if(resizeWidth)
                {
                    //left border
                    if (x >= winrect.left && x < winrect.left + border_width)
                    {
                        *result = HTLEFT;
                    }
                    //right border
                    if (x < winrect.right && x >= winrect.right - border_width)
                    {
                        *result = HTRIGHT;
                    }
                }
                if(resizeHeight)
                {
                    //bottom border
                    if (y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOM;
                    }
                    //top border
                    if (y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOP;
                    }
                }
                if(resizeWidth && resizeHeight)
                {
                    //bottom left corner
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOMLEFT;
                    }
                    //bottom right corner
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y < winrect.bottom && y >= winrect.bottom - border_width)
                    {
                        *result = HTBOTTOMRIGHT;
                    }
                    //top left corner
                    if (x >= winrect.left && x < winrect.left + border_width &&
                            y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOPLEFT;
                    }
                    //top right corner
                    if (x < winrect.right && x >= winrect.right - border_width &&
                            y >= winrect.top && y < winrect.top + border_width)
                    {
                        *result = HTTOPRIGHT;
                    }
                }
            }
            if (0!=*result) return true;

            //if not return false, you can't the menu when full screen
            if(m_widget->isFullScreen())
                return false;

            //*result still equals 0, that means the cursor locate OUTSIDE the frame area
            //but it may locate in titlebar area
            if (!m_title) return false;

            //support highdpi
            double dpr = m_widget->devicePixelRatioF();
            QPoint pos = m_title->mapFromGlobal(QPoint(x/dpr,y/dpr));

            if (!m_title->rect().contains(pos))
            {
                return false;
            }
            QWidget* child = m_title->childAt(pos);
            if (!child)
            {
                *result = HTCAPTION;
                return true;
            }else{
                if (m_whiteList.contains(child))
                {
                    *result = HTCAPTION;
                    return true;
                }
            }
            return false;
        } //end case WM_NCHITTEST
        case WM_GETMINMAXINFO:
        {
            *isProcessed = true;
            if (::IsZoomed(msg->hwnd)) {
                RECT frame = { 0, 0, 0, 0 };
                AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);

                //record frame area data
                double dpr = m_widget->devicePixelRatioF();

                m_frames.setLeft(abs(frame.left)/dpr+0.5);
                m_frames.setTop(abs(frame.bottom)/dpr+0.5);
                m_frames.setRight(abs(frame.right)/dpr+0.5);
                m_frames.setBottom(abs(frame.bottom)/dpr+0.5);

                setContentsMargins(m_margins);
                m_bJustMaximized = true;
            }else {
                if (m_bJustMaximized)
                {
                    m_widget->QWidget::setContentsMargins(m_margins);
                    m_frames = QMargins();
                    m_bJustMaximized = false;
                }
            }
            return false;
        }
        default:
        {
            *isProcessed = false;
            return false;
        }
        }
    }

//    void processFullScreen()
//    {

//    }

public slots:
    void onMaxNormal()
    {
        if(m_widget->isMaximized())
            m_widget->showNormal();
        else
            m_widget->showMaximized();
    }

    void onMin()
    {
        m_widget->showMinimized();
    }

    void onFull()
    {
        m_widget->showFullScreen();
    }

    void onClose()
    {
        m_widget->close();
    }

protected:
//    bool eventFilter
    bool eventFilter(QObject *watched, QEvent *evt) override
    {
        if(watched != m_widget) return false;

        int type = evt->type();
        switch( type )
        {
        case QEvent::WindowStateChange:
        {
            QWindowStateChangeEvent *sevt = static_cast<QWindowStateChangeEvent *>(evt);
            if(m_widget->isFullScreen())
            {
                m_title->hide();
                m_titleHeight = 0;
                setContentsMargins(m_margins);
                if (sevt->oldState() & Qt::WindowMaximized)
                {
                    m_frames = QMargins();
                }
            }
            else
            {
                m_title->show();
                m_titleHeight = m_title->height();
                setContentsMargins(m_margins);
            }

        }
            break;

        }

        return QObject::eventFilter(watched, evt);
    }

private slots:
    void onTitleBarDestroyed()
    {
        if (m_title == QObject::sender())
        {
            m_title = Q_NULLPTR;
        }
    }

private:
    QWidget *m_widget;
    FLTitle *m_title = nullptr;
    bool m_bResizeable;

    int m_titleHeight;
    int m_borderWidth;

    QMargins m_margins;
    QMargins m_frames;
    QList<QWidget*> m_whiteList;
    bool m_bJustMaximized;
};



FLTitle::FLTitle(QWidget *parent) :
    QWidget(parent)
{
    initActions();
    QWidget *tools;
    QHBoxLayout *titleBarHLayout, *toolsHLayout;
//    QToolButton *icon, *minBtn, *maxBtn, *closeBtn, *fullBtn;

    this->setObjectName(QStringLiteral("CFramelessWidget"));
    this->resize(800, 35);
    this->setObjectName(QStringLiteral("titlebar"));

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
    this->setSizePolicy(sizePolicy);
    this->setMinimumSize(QSize(0, titlebarHeight));
    this->setMaximumSize(QSize(16777215, titlebarHeight));

    titleBarHLayout = new QHBoxLayout(this);
    titleBarHLayout->setSpacing(0);
    titleBarHLayout->setObjectName(QStringLiteral("titleBarHLayout"));
    titleBarHLayout->setContentsMargins(0, 0, 0, 0);
    icon = new QToolButton(this);
    icon->setDefaultAction(m_actions[TAIcon]);
    icon->setObjectName(QStringLiteral("icon"));
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(icon->sizePolicy().hasHeightForWidth());
    icon->setSizePolicy(sizePolicy1);
    icon->setIconSize(QSize(titlebarHeight, titlebarHeight));
    icon->setMinimumSize(QSize(titlebarHeight, titlebarHeight));
    icon->setMaximumSize(QSize(titlebarHeight, titlebarHeight));

    titleBarHLayout->addWidget(icon);

    title = new QLabel(this);
    title->setObjectName(QStringLiteral("title"));

    titleBarHLayout->addWidget(title);

    tools = new QWidget(this);
    tools->setObjectName(QStringLiteral("tools"));
    sizePolicy1.setHeightForWidth(tools->sizePolicy().hasHeightForWidth());
    tools->setSizePolicy(sizePolicy1);
    tools->setMinimumSize(QSize(150, titlebarHeight));
    tools->setMaximumSize(QSize(150, titlebarHeight));

    toolsHLayout = new QHBoxLayout(tools);
    toolsHLayout->setSpacing(0);
    toolsHLayout->setObjectName(QStringLiteral("toolsHLayout"));
    toolsHLayout->setContentsMargins(0, 0, 0, 0);

    fullBtn = new QToolButton(tools);
    fullBtn->setDefaultAction(m_actions[TAFull]);
    fullBtn->setObjectName(QStringLiteral("full"));
    sizePolicy1.setHeightForWidth(fullBtn->sizePolicy().hasHeightForWidth());
    fullBtn->setSizePolicy(sizePolicy1);
    fullBtn->setMinimumSize(QSize(titlebarHeight+5, titlebarHeight));

    toolsHLayout->addWidget(fullBtn);

    minBtn = new QToolButton(tools);
    minBtn->setDefaultAction(m_actions[TAMin]);
    minBtn->setObjectName(QStringLiteral("min"));
    sizePolicy1.setHeightForWidth(minBtn->sizePolicy().hasHeightForWidth());
    minBtn->setSizePolicy(sizePolicy1);
    minBtn->setMinimumSize(QSize(titlebarHeight+5, titlebarHeight));

    toolsHLayout->addWidget(minBtn);

    maxBtn = new QToolButton(tools);
    maxBtn->setDefaultAction(m_actions[TAMaxNormal]);
    maxBtn->setObjectName(QStringLiteral("max"));
    sizePolicy1.setHeightForWidth(maxBtn->sizePolicy().hasHeightForWidth());
    maxBtn->setSizePolicy(sizePolicy1);
    maxBtn->setMinimumSize(QSize(titlebarHeight+5, titlebarHeight));

    toolsHLayout->addWidget(maxBtn);

    closeBtn = new QToolButton(tools);
    closeBtn->setDefaultAction(m_actions[TAClose]);
    closeBtn->setObjectName(QStringLiteral("close"));
    sizePolicy1.setHeightForWidth(closeBtn->sizePolicy().hasHeightForWidth());
    closeBtn->setSizePolicy(sizePolicy1);
    closeBtn->setMinimumSize(QSize(titlebarHeight+5, titlebarHeight));

    toolsHLayout->addWidget(closeBtn);

    toolsHLayout->setStretch(0, 1);
    toolsHLayout->setStretch(1, 1);
    toolsHLayout->setStretch(2, 1);
    toolsHLayout->setStretch(3, 1);

    titleBarHLayout->addWidget(tools);

    titleBarHLayout->setStretch(0, 1);
    titleBarHLayout->setStretch(1, 10);
    titleBarHLayout->setStretch(2, 1);

    QFont font;
    font.setFamily("Microsoft YaHei");
    title->setFont(font);

//    fullBtn->setCheckable(true);
    defaultStyle();
}

void FLTitle::defaultStyle()
{
    QString style = \
    "QToolButton"
    "{"
    " border:none;"
    "}"
    "QToolButton#min::hover, QToolButton#max::hover"
    "{"
    " background:#33aabb;"
    " color:white;"
    "}"
    "QToolButton#full::hover, QToolButton#full::checked"
    "{"
    " background:#77ffdd;"
    "}"
    "QToolButton#close::hover"
    "{"
    " background:red;"
    " color:white;"
    "}";
    this->setStyleSheet(style);
}

QList<QWidget *> FLTitle::ignoreWidgetList()
{
    QList<QWidget *> list = {
       title
    };
    return list;
}

void FLTitle::on_actions(FLTitle::TitleAction ta)
{
    m_actions[ta]->triggered();
}

void FLTitle::setIcon(const QString &str)
{
    m_actions[TAIcon]->setText(str);
}

void FLTitle::setIcon(const QIcon &icon)
{
    m_actions[TAIcon]->setIcon(icon);
}

void FLTitle::setTitle(const QString &text)
{
    title->setText(text);
}

void FLTitle::initActions()
{
    m_actions[TAMin]  = \
            new QAction(FLIcon->GetChar("window-minimize"));
    m_actions[TAMaxNormal]  = \
            new QAction(FLIcon->GetChar("window-restore"));
    m_actions[TAFull]  = \
            new QAction(FLIcon->GetChar(QChar(0xf26c)));
    m_actions[TAClose]  = \
            new QAction(FLIcon->GetChar("window-close"));
    m_actions[TAIcon]  = \
            new QAction(QChar(0xf555));

    for(auto it : m_actions)
    {
        it.second->setFont(FLIcon->font());
    }
    connect(m_actions[TAMaxNormal], &QAction::triggered, this, &FLTitle::clk_maxnormal);
    connect(m_actions[TAMin], &QAction::triggered, this, &FLTitle::clk_min);
    connect(m_actions[TAFull], &QAction::triggered, this, &FLTitle::clk_full);
    connect(m_actions[TAClose], &QAction::triggered, this, &FLTitle::clk_close);
    connect(m_actions[TAIcon], &QAction::triggered, this, &FLTitle::clk_icon);
}


FLWidget::FLWidget(QWidget *parent) :
    QWidget(parent) ,
    m_flHandle(new FLHandle(this))
{
    this->resize(800, 600);

    setTitle(new FLTitle(), 35);

    m_flHandle->init();
}

void FLWidget::setResizeable(bool resizeable)
{
    m_flHandle->setResizeable(resizeable);
}

bool FLWidget::isResizeable()
{
    return m_flHandle->isResizeable();
}

void FLWidget::setResizeableAreaWidth(int width)
{
    m_flHandle->setResizeableAreaWidth(width);
}

void FLWidget::setTitle(FLTitle *title, int titleHeight)
{
    m_flHandle->setTitle(title, titleHeight);
}

void FLWidget::setContentsMargins(int left, int top, int right, int bottom)
{
    m_flHandle->setContentsMargins(left, top, right, bottom);
}

void FLWidget::setContentsMargins(const QMargins &margins)
{
    m_flHandle->setContentsMargins(margins);
}

void FLWidget::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    m_flHandle->getContentsMargins(left, top, right, bottom);
}

QMargins FLWidget::contentsMargins() const
{
    return m_flHandle->contentsMargins();
}

QRect FLWidget::contentsRect() const
{
    return m_flHandle->contentsRect();
}

void FLWidget::showFullScreen()
{
    QWidget::showFullScreen();
}

void FLWidget::restoreScreen()
{
    QWidget::showMaximized();
}

void FLWidget::resizeEvent(QResizeEvent *event)
{
    m_flHandle->resizeEvent(event);
    return QWidget::resizeEvent(event);
}

bool FLWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    bool bProcessed;
    bool res;
    res = m_flHandle->handleMessage(message, result, &bProcessed);
    if(bProcessed)
        return res;
    else
        return QWidget::nativeEvent(eventType, message, result);
}



FLMainWindow::FLMainWindow(QWidget *parent) :
    QMainWindow(parent) ,
    m_flHandle(new FLHandle(this))
{
    this->resize(800, 600);

    setTitle(new FLTitle(), 35);
    m_flHandle->init();
}

void FLMainWindow::setResizeable(bool resizeable)
{
    m_flHandle->setResizeable(resizeable);
}

bool FLMainWindow::isResizeable()
{
    return m_flHandle->isResizeable();
}

void FLMainWindow::setResizeableAreaWidth(int width)
{
    m_flHandle->setResizeableAreaWidth(width);
}

void FLMainWindow::setTitle(FLTitle *title, int titleHeight)
{
    m_flHandle->setTitle(title, titleHeight);
}

void FLMainWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    m_flHandle->setContentsMargins(left, top, right, bottom);
}

void FLMainWindow::setContentsMargins(const QMargins &margins)
{
    m_flHandle->setContentsMargins(margins);
}

void FLMainWindow::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    m_flHandle->getContentsMargins(left, top, right, bottom);
}

QMargins FLMainWindow::contentsMargins() const
{
    return m_flHandle->contentsMargins();
}

QRect FLMainWindow::contentsRect() const
{
    return m_flHandle->contentsRect();
}

void FLMainWindow::showFullScreen()
{
    if(isFullScreen())
        return;
    m_flHandle->onFull();
}

void FLMainWindow::restoreScreen()
{
    if(!isFullScreen())
        return;
    m_flHandle->onMaxNormal();
}

void FLMainWindow::resizeEvent(QResizeEvent *event)
{
    m_flHandle->resizeEvent(event);
    return QWidget::resizeEvent(event);
}

bool FLMainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    bool bProcessed;
    bool res;
    res = m_flHandle->handleMessage(message, result, &bProcessed);
    if(bProcessed)
        return res;
    else
        return QWidget::nativeEvent(eventType, message, result);
}



#include "flwidget.moc"
#endif
