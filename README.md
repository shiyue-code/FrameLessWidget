# FrameLessWidget
Qt FrameLess Widget for msvc2019

Coding as follows：

```cpp
#include "flwidget.h"
class MainWindow : public FLMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
```

# Example
![e1](./frameLessShow1.png)
