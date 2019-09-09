/*
    Precompiled Header
    ------------------

    This file is for speeding up Qt build time.
    This PCH allows you to specify, which files are commonly used.
    The compiler can pre-compile them before a build starts 
    and then use the resulting information when compiling each .cpp.

    *Usage*:
                                    
    # use a precompiled header file for faster compilation 
    PRECOMPILED_HEADER = ../../build/pch/precompiled_header.h
    CONFIG += precompile_header

    *WARNING*: Do not include this file explicitly in your project.
*/
    #include <QAction>
    #include <QButtonGroup>
    #include <QCheckBox>
    #include <QComboBox>
    #include <QDialogButtonBox>
    #include <QDrag>
    #include <QFileDialog>
    #include <QFrame>
    #include <QHBoxLayout>
    #include <QHeaderView>
    #include <QLabel>
    #include <QLineEdit>
    #include <QMimeData>
    #include <QMouseEvent>
    #include <QPushButton>
    #include <QRadioButton>
    #include <QRegularExpression>
    #include <QSpacerItem>
    #include <QStackedWidget>
    #include <QStyle>
    #include <QSystemTrayIcon>
    #include <QTabWidget>
    #include <QTimer>
    #include <QToolButton>
    #include <QToolTip>
    #include <QValidator>
    #include <QVBoxLayout>

    #include <QApplication>
    #include <QColor>
    #include <QDir>
    #include <QMap>
    #include <QPointer>
    #include <QStringList>
    #include <QVariant>
    #include <QVector>

    #include <algorithm>
    #include <array>
    #include <bitset>
    #include <cassert>
    #include <cfloat>
    #include <cmath>
    #include <complex>
    #include <condition_variable>
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <functional>
    #include <initializer_list>
    #include <iterator>
    #include <map>
    #include <memory>
    #include <mutex>
    #include <numeric>
    #include <set>
    #include <string>
    #include <thread>
    #include <type_traits>
    #include <unordered_map>
    #include <vector>