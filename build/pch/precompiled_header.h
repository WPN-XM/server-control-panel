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
    #include <QFileDialog>
    #include <QPushButton>
    #include <QLabel>
    #include <QCheckBox>
    #include <QComboBox>
    #include <QVBoxLayout>
    #include <QHBoxLayout>
    #include <QDrag>
    #include <QMimeData>
    #include <QToolButton>
    #include <QFrame>
    #include <QValidator>
    #include <QAction>
    #include <QButtonGroup>
    #include <QHeaderView>
    #include <QLineEdit>
    #include <QSpacerItem>
    #include <QStackedWidget>
    #include <QRadioButton>
    #include <QTabWidget>
    #include <QToolTip>
    #include <QMouseEvent>
    #include <QStyle>
    #include <QTimer>

    #include <QApplication>
    #include <QVariant>
    #include <QMap>
    #include <QVector>
    #include <QStringList>
    #include <QDir>
    #include <QPointer>
    #include <QColor>

    #include <string>
    #include <set>
    #include <map>
    #include <memory>
    #include <vector>
    #include <unordered_map>
    #include <array>
    #include <bitset>
    #include <initializer_list>
    #include <functional>
    #include <algorithm>
    #include <numeric>
    #include <iterator>
    #include <type_traits>
    #include <cmath>
    #include <cassert>
    #include <cfloat>
    #include <complex>
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <mutex>
    #include <thread>
    #include <condition_variable>