/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../Sparky/src/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[144];
    char stringdata0[2969];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 15), // "connectionError"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 3), // "msg"
QT_MOC_LITERAL(4, 32, 15), // "onRtuPortActive"
QT_MOC_LITERAL(5, 48, 17), // "onRtuPortActive_2"
QT_MOC_LITERAL(6, 66, 17), // "onRtuPortActive_3"
QT_MOC_LITERAL(7, 84, 17), // "onRtuPortActive_4"
QT_MOC_LITERAL(8, 102, 17), // "onRtuPortActive_5"
QT_MOC_LITERAL(9, 120, 17), // "onRtuPortActive_6"
QT_MOC_LITERAL(10, 138, 16), // "changeSerialPort"
QT_MOC_LITERAL(11, 155, 18), // "changeSerialPort_2"
QT_MOC_LITERAL(12, 174, 18), // "changeSerialPort_3"
QT_MOC_LITERAL(13, 193, 18), // "changeSerialPort_4"
QT_MOC_LITERAL(14, 212, 18), // "changeSerialPort_5"
QT_MOC_LITERAL(15, 231, 18), // "changeSerialPort_6"
QT_MOC_LITERAL(16, 250, 15), // "calibration_1_1"
QT_MOC_LITERAL(17, 266, 15), // "calibration_1_2"
QT_MOC_LITERAL(18, 282, 15), // "calibration_1_3"
QT_MOC_LITERAL(19, 298, 15), // "calibration_2_1"
QT_MOC_LITERAL(20, 314, 15), // "calibration_2_2"
QT_MOC_LITERAL(21, 330, 15), // "calibration_2_3"
QT_MOC_LITERAL(22, 346, 15), // "calibration_3_1"
QT_MOC_LITERAL(23, 362, 15), // "calibration_3_2"
QT_MOC_LITERAL(24, 378, 15), // "calibration_3_3"
QT_MOC_LITERAL(25, 394, 15), // "calibration_4_1"
QT_MOC_LITERAL(26, 410, 15), // "calibration_4_2"
QT_MOC_LITERAL(27, 426, 15), // "calibration_4_3"
QT_MOC_LITERAL(28, 442, 15), // "calibration_5_1"
QT_MOC_LITERAL(29, 458, 15), // "calibration_5_2"
QT_MOC_LITERAL(30, 474, 15), // "calibration_5_3"
QT_MOC_LITERAL(31, 490, 15), // "calibration_6_1"
QT_MOC_LITERAL(32, 506, 15), // "calibration_6_2"
QT_MOC_LITERAL(33, 522, 15), // "calibration_6_3"
QT_MOC_LITERAL(34, 538, 22), // "initializeToolbarIcons"
QT_MOC_LITERAL(35, 561, 23), // "initializePressureGauge"
QT_MOC_LITERAL(36, 585, 26), // "initializeTemperatureGauge"
QT_MOC_LITERAL(37, 612, 22), // "initializeDensityGauge"
QT_MOC_LITERAL(38, 635, 17), // "initializeRPGauge"
QT_MOC_LITERAL(39, 653, 16), // "onLoopTabChanged"
QT_MOC_LITERAL(40, 670, 13), // "clearMonitors"
QT_MOC_LITERAL(41, 684, 20), // "updateRequestPreview"
QT_MOC_LITERAL(42, 705, 18), // "updateRegisterView"
QT_MOC_LITERAL(43, 724, 13), // "enableHexView"
QT_MOC_LITERAL(44, 738, 17), // "sendModbusRequest"
QT_MOC_LITERAL(45, 756, 17), // "onSendButtonPress"
QT_MOC_LITERAL(46, 774, 16), // "pollForDataOnBus"
QT_MOC_LITERAL(47, 791, 18), // "openBatchProcessor"
QT_MOC_LITERAL(48, 810, 12), // "aboutQModBus"
QT_MOC_LITERAL(49, 823, 17), // "onCheckBoxChecked"
QT_MOC_LITERAL(50, 841, 11), // "resetStatus"
QT_MOC_LITERAL(51, 853, 14), // "setStatusError"
QT_MOC_LITERAL(52, 868, 19), // "updatePressureGauge"
QT_MOC_LITERAL(53, 888, 22), // "updateTemperatureGauge"
QT_MOC_LITERAL(54, 911, 18), // "updateDensityGauge"
QT_MOC_LITERAL(55, 930, 13), // "updateRPGauge"
QT_MOC_LITERAL(56, 944, 20), // "onFloatButtonPressed"
QT_MOC_LITERAL(57, 965, 22), // "onIntegerButtonPressed"
QT_MOC_LITERAL(58, 988, 19), // "onCoilButtonPressed"
QT_MOC_LITERAL(59, 1008, 19), // "onReadButtonPressed"
QT_MOC_LITERAL(60, 1028, 20), // "onWriteButtonPressed"
QT_MOC_LITERAL(61, 1049, 23), // "onEquationButtonPressed"
QT_MOC_LITERAL(62, 1073, 11), // "loadCsvFile"
QT_MOC_LITERAL(63, 1085, 15), // "loadCsvTemplate"
QT_MOC_LITERAL(64, 1101, 16), // "onUploadEquation"
QT_MOC_LITERAL(65, 1118, 18), // "onDownloadEquation"
QT_MOC_LITERAL(66, 1137, 15), // "updateRegisters"
QT_MOC_LITERAL(67, 1153, 19), // "onProductBtnPressed"
QT_MOC_LITERAL(68, 1173, 23), // "onDownloadButtonChecked"
QT_MOC_LITERAL(69, 1197, 11), // "saveCsvFile"
QT_MOC_LITERAL(70, 1209, 23), // "setupCalibrationRequest"
QT_MOC_LITERAL(71, 1233, 20), // "onRadioButtonPressed"
QT_MOC_LITERAL(72, 1254, 22), // "onRadioButton_2Pressed"
QT_MOC_LITERAL(73, 1277, 22), // "onRadioButton_3Pressed"
QT_MOC_LITERAL(74, 1300, 22), // "onRadioButton_4Pressed"
QT_MOC_LITERAL(75, 1323, 22), // "onRadioButton_7Pressed"
QT_MOC_LITERAL(76, 1346, 22), // "onRadioButton_8Pressed"
QT_MOC_LITERAL(77, 1369, 22), // "onRadioButton_9Pressed"
QT_MOC_LITERAL(78, 1392, 23), // "onRadioButton_10Pressed"
QT_MOC_LITERAL(79, 1416, 23), // "onRadioButton_13Pressed"
QT_MOC_LITERAL(80, 1440, 23), // "onRadioButton_14Pressed"
QT_MOC_LITERAL(81, 1464, 23), // "onRadioButton_15Pressed"
QT_MOC_LITERAL(82, 1488, 23), // "onRadioButton_16Pressed"
QT_MOC_LITERAL(83, 1512, 23), // "onRadioButton_19Pressed"
QT_MOC_LITERAL(84, 1536, 23), // "onRadioButton_20Pressed"
QT_MOC_LITERAL(85, 1560, 23), // "onRadioButton_23Pressed"
QT_MOC_LITERAL(86, 1584, 23), // "onRadioButton_24Pressed"
QT_MOC_LITERAL(87, 1608, 23), // "onRadioButton_25Pressed"
QT_MOC_LITERAL(88, 1632, 23), // "onRadioButton_26Pressed"
QT_MOC_LITERAL(89, 1656, 23), // "onRadioButton_27Pressed"
QT_MOC_LITERAL(90, 1680, 23), // "onRadioButton_28Pressed"
QT_MOC_LITERAL(91, 1704, 23), // "onRadioButton_31Pressed"
QT_MOC_LITERAL(92, 1728, 23), // "onRadioButton_32Pressed"
QT_MOC_LITERAL(93, 1752, 23), // "onRadioButton_33Pressed"
QT_MOC_LITERAL(94, 1776, 23), // "onRadioButton_34Pressed"
QT_MOC_LITERAL(95, 1800, 23), // "onRadioButton_37Pressed"
QT_MOC_LITERAL(96, 1824, 23), // "onRadioButton_38Pressed"
QT_MOC_LITERAL(97, 1848, 23), // "onRadioButton_41Pressed"
QT_MOC_LITERAL(98, 1872, 23), // "onRadioButton_42Pressed"
QT_MOC_LITERAL(99, 1896, 23), // "onRadioButton_43Pressed"
QT_MOC_LITERAL(100, 1920, 23), // "onRadioButton_44Pressed"
QT_MOC_LITERAL(101, 1944, 23), // "onRadioButton_45Pressed"
QT_MOC_LITERAL(102, 1968, 23), // "onRadioButton_46Pressed"
QT_MOC_LITERAL(103, 1992, 23), // "onRadioButton_49Pressed"
QT_MOC_LITERAL(104, 2016, 23), // "onRadioButton_50Pressed"
QT_MOC_LITERAL(105, 2040, 23), // "onRadioButton_51Pressed"
QT_MOC_LITERAL(106, 2064, 23), // "onRadioButton_52Pressed"
QT_MOC_LITERAL(107, 2088, 23), // "onRadioButton_55Pressed"
QT_MOC_LITERAL(108, 2112, 23), // "onRadioButton_56Pressed"
QT_MOC_LITERAL(109, 2136, 23), // "onRadioButton_59Pressed"
QT_MOC_LITERAL(110, 2160, 23), // "onRadioButton_60Pressed"
QT_MOC_LITERAL(111, 2184, 23), // "onRadioButton_61Pressed"
QT_MOC_LITERAL(112, 2208, 23), // "onRadioButton_62Pressed"
QT_MOC_LITERAL(113, 2232, 23), // "onRadioButton_63Pressed"
QT_MOC_LITERAL(114, 2256, 23), // "onRadioButton_64Pressed"
QT_MOC_LITERAL(115, 2280, 23), // "onRadioButton_67Pressed"
QT_MOC_LITERAL(116, 2304, 23), // "onRadioButton_68Pressed"
QT_MOC_LITERAL(117, 2328, 23), // "onRadioButton_69Pressed"
QT_MOC_LITERAL(118, 2352, 23), // "onRadioButton_70Pressed"
QT_MOC_LITERAL(119, 2376, 23), // "onRadioButton_73Pressed"
QT_MOC_LITERAL(120, 2400, 23), // "onRadioButton_74Pressed"
QT_MOC_LITERAL(121, 2424, 23), // "onRadioButton_77Pressed"
QT_MOC_LITERAL(122, 2448, 23), // "onRadioButton_78Pressed"
QT_MOC_LITERAL(123, 2472, 23), // "onRadioButton_79Pressed"
QT_MOC_LITERAL(124, 2496, 23), // "onRadioButton_80Pressed"
QT_MOC_LITERAL(125, 2520, 23), // "onRadioButton_81Pressed"
QT_MOC_LITERAL(126, 2544, 23), // "onRadioButton_82Pressed"
QT_MOC_LITERAL(127, 2568, 23), // "onRadioButton_85Pressed"
QT_MOC_LITERAL(128, 2592, 23), // "onRadioButton_86Pressed"
QT_MOC_LITERAL(129, 2616, 23), // "onRadioButton_87Pressed"
QT_MOC_LITERAL(130, 2640, 23), // "onRadioButton_88Pressed"
QT_MOC_LITERAL(131, 2664, 23), // "onRadioButton_91Pressed"
QT_MOC_LITERAL(132, 2688, 23), // "onRadioButton_92Pressed"
QT_MOC_LITERAL(133, 2712, 23), // "onRadioButton_95Pressed"
QT_MOC_LITERAL(134, 2736, 23), // "onRadioButton_96Pressed"
QT_MOC_LITERAL(135, 2760, 23), // "onRadioButton_97Pressed"
QT_MOC_LITERAL(136, 2784, 23), // "onRadioButton_98Pressed"
QT_MOC_LITERAL(137, 2808, 23), // "onRadioButton_99Pressed"
QT_MOC_LITERAL(138, 2832, 24), // "onRadioButton_100Pressed"
QT_MOC_LITERAL(139, 2857, 24), // "onRadioButton_103Pressed"
QT_MOC_LITERAL(140, 2882, 24), // "onRadioButton_104Pressed"
QT_MOC_LITERAL(141, 2907, 24), // "onRadioButton_105Pressed"
QT_MOC_LITERAL(142, 2932, 24), // "onRadioButton_106Pressed"
QT_MOC_LITERAL(143, 2957, 11) // "updateGraph"

    },
    "MainWindow\0connectionError\0\0msg\0"
    "onRtuPortActive\0onRtuPortActive_2\0"
    "onRtuPortActive_3\0onRtuPortActive_4\0"
    "onRtuPortActive_5\0onRtuPortActive_6\0"
    "changeSerialPort\0changeSerialPort_2\0"
    "changeSerialPort_3\0changeSerialPort_4\0"
    "changeSerialPort_5\0changeSerialPort_6\0"
    "calibration_1_1\0calibration_1_2\0"
    "calibration_1_3\0calibration_2_1\0"
    "calibration_2_2\0calibration_2_3\0"
    "calibration_3_1\0calibration_3_2\0"
    "calibration_3_3\0calibration_4_1\0"
    "calibration_4_2\0calibration_4_3\0"
    "calibration_5_1\0calibration_5_2\0"
    "calibration_5_3\0calibration_6_1\0"
    "calibration_6_2\0calibration_6_3\0"
    "initializeToolbarIcons\0initializePressureGauge\0"
    "initializeTemperatureGauge\0"
    "initializeDensityGauge\0initializeRPGauge\0"
    "onLoopTabChanged\0clearMonitors\0"
    "updateRequestPreview\0updateRegisterView\0"
    "enableHexView\0sendModbusRequest\0"
    "onSendButtonPress\0pollForDataOnBus\0"
    "openBatchProcessor\0aboutQModBus\0"
    "onCheckBoxChecked\0resetStatus\0"
    "setStatusError\0updatePressureGauge\0"
    "updateTemperatureGauge\0updateDensityGauge\0"
    "updateRPGauge\0onFloatButtonPressed\0"
    "onIntegerButtonPressed\0onCoilButtonPressed\0"
    "onReadButtonPressed\0onWriteButtonPressed\0"
    "onEquationButtonPressed\0loadCsvFile\0"
    "loadCsvTemplate\0onUploadEquation\0"
    "onDownloadEquation\0updateRegisters\0"
    "onProductBtnPressed\0onDownloadButtonChecked\0"
    "saveCsvFile\0setupCalibrationRequest\0"
    "onRadioButtonPressed\0onRadioButton_2Pressed\0"
    "onRadioButton_3Pressed\0onRadioButton_4Pressed\0"
    "onRadioButton_7Pressed\0onRadioButton_8Pressed\0"
    "onRadioButton_9Pressed\0onRadioButton_10Pressed\0"
    "onRadioButton_13Pressed\0onRadioButton_14Pressed\0"
    "onRadioButton_15Pressed\0onRadioButton_16Pressed\0"
    "onRadioButton_19Pressed\0onRadioButton_20Pressed\0"
    "onRadioButton_23Pressed\0onRadioButton_24Pressed\0"
    "onRadioButton_25Pressed\0onRadioButton_26Pressed\0"
    "onRadioButton_27Pressed\0onRadioButton_28Pressed\0"
    "onRadioButton_31Pressed\0onRadioButton_32Pressed\0"
    "onRadioButton_33Pressed\0onRadioButton_34Pressed\0"
    "onRadioButton_37Pressed\0onRadioButton_38Pressed\0"
    "onRadioButton_41Pressed\0onRadioButton_42Pressed\0"
    "onRadioButton_43Pressed\0onRadioButton_44Pressed\0"
    "onRadioButton_45Pressed\0onRadioButton_46Pressed\0"
    "onRadioButton_49Pressed\0onRadioButton_50Pressed\0"
    "onRadioButton_51Pressed\0onRadioButton_52Pressed\0"
    "onRadioButton_55Pressed\0onRadioButton_56Pressed\0"
    "onRadioButton_59Pressed\0onRadioButton_60Pressed\0"
    "onRadioButton_61Pressed\0onRadioButton_62Pressed\0"
    "onRadioButton_63Pressed\0onRadioButton_64Pressed\0"
    "onRadioButton_67Pressed\0onRadioButton_68Pressed\0"
    "onRadioButton_69Pressed\0onRadioButton_70Pressed\0"
    "onRadioButton_73Pressed\0onRadioButton_74Pressed\0"
    "onRadioButton_77Pressed\0onRadioButton_78Pressed\0"
    "onRadioButton_79Pressed\0onRadioButton_80Pressed\0"
    "onRadioButton_81Pressed\0onRadioButton_82Pressed\0"
    "onRadioButton_85Pressed\0onRadioButton_86Pressed\0"
    "onRadioButton_87Pressed\0onRadioButton_88Pressed\0"
    "onRadioButton_91Pressed\0onRadioButton_92Pressed\0"
    "onRadioButton_95Pressed\0onRadioButton_96Pressed\0"
    "onRadioButton_97Pressed\0onRadioButton_98Pressed\0"
    "onRadioButton_99Pressed\0"
    "onRadioButton_100Pressed\0"
    "onRadioButton_103Pressed\0"
    "onRadioButton_104Pressed\0"
    "onRadioButton_105Pressed\0"
    "onRadioButton_106Pressed\0updateGraph"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     141,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  719,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,  722,    2, 0x08 /* Private */,
       5,    1,  725,    2, 0x08 /* Private */,
       6,    1,  728,    2, 0x08 /* Private */,
       7,    1,  731,    2, 0x08 /* Private */,
       8,    1,  734,    2, 0x08 /* Private */,
       9,    1,  737,    2, 0x08 /* Private */,
      10,    1,  740,    2, 0x08 /* Private */,
      11,    1,  743,    2, 0x08 /* Private */,
      12,    1,  746,    2, 0x08 /* Private */,
      13,    1,  749,    2, 0x08 /* Private */,
      14,    1,  752,    2, 0x08 /* Private */,
      15,    1,  755,    2, 0x08 /* Private */,
      16,    0,  758,    2, 0x08 /* Private */,
      17,    0,  759,    2, 0x08 /* Private */,
      18,    0,  760,    2, 0x08 /* Private */,
      19,    0,  761,    2, 0x08 /* Private */,
      20,    0,  762,    2, 0x08 /* Private */,
      21,    0,  763,    2, 0x08 /* Private */,
      22,    0,  764,    2, 0x08 /* Private */,
      23,    0,  765,    2, 0x08 /* Private */,
      24,    0,  766,    2, 0x08 /* Private */,
      25,    0,  767,    2, 0x08 /* Private */,
      26,    0,  768,    2, 0x08 /* Private */,
      27,    0,  769,    2, 0x08 /* Private */,
      28,    0,  770,    2, 0x08 /* Private */,
      29,    0,  771,    2, 0x08 /* Private */,
      30,    0,  772,    2, 0x08 /* Private */,
      31,    0,  773,    2, 0x08 /* Private */,
      32,    0,  774,    2, 0x08 /* Private */,
      33,    0,  775,    2, 0x08 /* Private */,
      34,    0,  776,    2, 0x08 /* Private */,
      35,    0,  777,    2, 0x08 /* Private */,
      36,    0,  778,    2, 0x08 /* Private */,
      37,    0,  779,    2, 0x08 /* Private */,
      38,    0,  780,    2, 0x08 /* Private */,
      39,    1,  781,    2, 0x08 /* Private */,
      40,    0,  784,    2, 0x08 /* Private */,
      41,    0,  785,    2, 0x08 /* Private */,
      42,    0,  786,    2, 0x08 /* Private */,
      43,    0,  787,    2, 0x08 /* Private */,
      44,    0,  788,    2, 0x08 /* Private */,
      45,    0,  789,    2, 0x08 /* Private */,
      46,    0,  790,    2, 0x08 /* Private */,
      47,    0,  791,    2, 0x08 /* Private */,
      48,    0,  792,    2, 0x08 /* Private */,
      49,    1,  793,    2, 0x08 /* Private */,
      50,    0,  796,    2, 0x08 /* Private */,
      51,    1,  797,    2, 0x08 /* Private */,
      52,    0,  800,    2, 0x08 /* Private */,
      53,    0,  801,    2, 0x08 /* Private */,
      54,    0,  802,    2, 0x08 /* Private */,
      55,    0,  803,    2, 0x08 /* Private */,
      56,    1,  804,    2, 0x08 /* Private */,
      57,    1,  807,    2, 0x08 /* Private */,
      58,    1,  810,    2, 0x08 /* Private */,
      59,    1,  813,    2, 0x08 /* Private */,
      60,    1,  816,    2, 0x08 /* Private */,
      61,    0,  819,    2, 0x08 /* Private */,
      62,    0,  820,    2, 0x08 /* Private */,
      63,    0,  821,    2, 0x08 /* Private */,
      64,    0,  822,    2, 0x08 /* Private */,
      65,    0,  823,    2, 0x08 /* Private */,
      66,    2,  824,    2, 0x08 /* Private */,
      67,    0,  829,    2, 0x08 /* Private */,
      68,    1,  830,    2, 0x08 /* Private */,
      69,    0,  833,    2, 0x08 /* Private */,
      70,    0,  834,    2, 0x08 /* Private */,
      71,    0,  835,    2, 0x08 /* Private */,
      72,    0,  836,    2, 0x08 /* Private */,
      73,    0,  837,    2, 0x08 /* Private */,
      74,    0,  838,    2, 0x08 /* Private */,
      75,    0,  839,    2, 0x08 /* Private */,
      76,    0,  840,    2, 0x08 /* Private */,
      77,    0,  841,    2, 0x08 /* Private */,
      78,    0,  842,    2, 0x08 /* Private */,
      79,    0,  843,    2, 0x08 /* Private */,
      80,    0,  844,    2, 0x08 /* Private */,
      81,    0,  845,    2, 0x08 /* Private */,
      82,    0,  846,    2, 0x08 /* Private */,
      83,    0,  847,    2, 0x08 /* Private */,
      84,    0,  848,    2, 0x08 /* Private */,
      85,    0,  849,    2, 0x08 /* Private */,
      86,    0,  850,    2, 0x08 /* Private */,
      87,    0,  851,    2, 0x08 /* Private */,
      88,    0,  852,    2, 0x08 /* Private */,
      89,    0,  853,    2, 0x08 /* Private */,
      90,    0,  854,    2, 0x08 /* Private */,
      91,    0,  855,    2, 0x08 /* Private */,
      92,    0,  856,    2, 0x08 /* Private */,
      93,    0,  857,    2, 0x08 /* Private */,
      94,    0,  858,    2, 0x08 /* Private */,
      95,    0,  859,    2, 0x08 /* Private */,
      96,    0,  860,    2, 0x08 /* Private */,
      97,    0,  861,    2, 0x08 /* Private */,
      98,    0,  862,    2, 0x08 /* Private */,
      99,    0,  863,    2, 0x08 /* Private */,
     100,    0,  864,    2, 0x08 /* Private */,
     101,    0,  865,    2, 0x08 /* Private */,
     102,    0,  866,    2, 0x08 /* Private */,
     103,    0,  867,    2, 0x08 /* Private */,
     104,    0,  868,    2, 0x08 /* Private */,
     105,    0,  869,    2, 0x08 /* Private */,
     106,    0,  870,    2, 0x08 /* Private */,
     107,    0,  871,    2, 0x08 /* Private */,
     108,    0,  872,    2, 0x08 /* Private */,
     109,    0,  873,    2, 0x08 /* Private */,
     110,    0,  874,    2, 0x08 /* Private */,
     111,    0,  875,    2, 0x08 /* Private */,
     112,    0,  876,    2, 0x08 /* Private */,
     113,    0,  877,    2, 0x08 /* Private */,
     114,    0,  878,    2, 0x08 /* Private */,
     115,    0,  879,    2, 0x08 /* Private */,
     116,    0,  880,    2, 0x08 /* Private */,
     117,    0,  881,    2, 0x08 /* Private */,
     118,    0,  882,    2, 0x08 /* Private */,
     119,    0,  883,    2, 0x08 /* Private */,
     120,    0,  884,    2, 0x08 /* Private */,
     121,    0,  885,    2, 0x08 /* Private */,
     122,    0,  886,    2, 0x08 /* Private */,
     123,    0,  887,    2, 0x08 /* Private */,
     124,    0,  888,    2, 0x08 /* Private */,
     125,    0,  889,    2, 0x08 /* Private */,
     126,    0,  890,    2, 0x08 /* Private */,
     127,    0,  891,    2, 0x08 /* Private */,
     128,    0,  892,    2, 0x08 /* Private */,
     129,    0,  893,    2, 0x08 /* Private */,
     130,    0,  894,    2, 0x08 /* Private */,
     131,    0,  895,    2, 0x08 /* Private */,
     132,    0,  896,    2, 0x08 /* Private */,
     133,    0,  897,    2, 0x08 /* Private */,
     134,    0,  898,    2, 0x08 /* Private */,
     135,    0,  899,    2, 0x08 /* Private */,
     136,    0,  900,    2, 0x08 /* Private */,
     137,    0,  901,    2, 0x08 /* Private */,
     138,    0,  902,    2, 0x08 /* Private */,
     139,    0,  903,    2, 0x08 /* Private */,
     140,    0,  904,    2, 0x08 /* Private */,
     141,    0,  905,    2, 0x08 /* Private */,
     142,    0,  906,    2, 0x08 /* Private */,
     143,    0,  907,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::Int,    2,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onRtuPortActive((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->onRtuPortActive_2((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onRtuPortActive_3((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->onRtuPortActive_4((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onRtuPortActive_5((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->onRtuPortActive_6((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->changeSerialPort((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->changeSerialPort_2((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->changeSerialPort_3((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->changeSerialPort_4((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->changeSerialPort_5((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->changeSerialPort_6((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->calibration_1_1(); break;
        case 14: _t->calibration_1_2(); break;
        case 15: _t->calibration_1_3(); break;
        case 16: _t->calibration_2_1(); break;
        case 17: _t->calibration_2_2(); break;
        case 18: _t->calibration_2_3(); break;
        case 19: _t->calibration_3_1(); break;
        case 20: _t->calibration_3_2(); break;
        case 21: _t->calibration_3_3(); break;
        case 22: _t->calibration_4_1(); break;
        case 23: _t->calibration_4_2(); break;
        case 24: _t->calibration_4_3(); break;
        case 25: _t->calibration_5_1(); break;
        case 26: _t->calibration_5_2(); break;
        case 27: _t->calibration_5_3(); break;
        case 28: _t->calibration_6_1(); break;
        case 29: _t->calibration_6_2(); break;
        case 30: _t->calibration_6_3(); break;
        case 31: _t->initializeToolbarIcons(); break;
        case 32: _t->initializePressureGauge(); break;
        case 33: _t->initializeTemperatureGauge(); break;
        case 34: _t->initializeDensityGauge(); break;
        case 35: _t->initializeRPGauge(); break;
        case 36: _t->onLoopTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 37: _t->clearMonitors(); break;
        case 38: _t->updateRequestPreview(); break;
        case 39: _t->updateRegisterView(); break;
        case 40: _t->enableHexView(); break;
        case 41: _t->sendModbusRequest(); break;
        case 42: _t->onSendButtonPress(); break;
        case 43: _t->pollForDataOnBus(); break;
        case 44: _t->openBatchProcessor(); break;
        case 45: _t->aboutQModBus(); break;
        case 46: _t->onCheckBoxChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 47: _t->resetStatus(); break;
        case 48: _t->setStatusError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 49: _t->updatePressureGauge(); break;
        case 50: _t->updateTemperatureGauge(); break;
        case 51: _t->updateDensityGauge(); break;
        case 52: _t->updateRPGauge(); break;
        case 53: _t->onFloatButtonPressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 54: _t->onIntegerButtonPressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 55: _t->onCoilButtonPressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 56: _t->onReadButtonPressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 57: _t->onWriteButtonPressed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 58: _t->onEquationButtonPressed(); break;
        case 59: _t->loadCsvFile(); break;
        case 60: _t->loadCsvTemplate(); break;
        case 61: _t->onUploadEquation(); break;
        case 62: _t->onDownloadEquation(); break;
        case 63: _t->updateRegisters((*reinterpret_cast< const bool(*)>(_a[1])),(*reinterpret_cast< const int(*)>(_a[2]))); break;
        case 64: _t->onProductBtnPressed(); break;
        case 65: _t->onDownloadButtonChecked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 66: _t->saveCsvFile(); break;
        case 67: _t->setupCalibrationRequest(); break;
        case 68: _t->onRadioButtonPressed(); break;
        case 69: _t->onRadioButton_2Pressed(); break;
        case 70: _t->onRadioButton_3Pressed(); break;
        case 71: _t->onRadioButton_4Pressed(); break;
        case 72: _t->onRadioButton_7Pressed(); break;
        case 73: _t->onRadioButton_8Pressed(); break;
        case 74: _t->onRadioButton_9Pressed(); break;
        case 75: _t->onRadioButton_10Pressed(); break;
        case 76: _t->onRadioButton_13Pressed(); break;
        case 77: _t->onRadioButton_14Pressed(); break;
        case 78: _t->onRadioButton_15Pressed(); break;
        case 79: _t->onRadioButton_16Pressed(); break;
        case 80: _t->onRadioButton_19Pressed(); break;
        case 81: _t->onRadioButton_20Pressed(); break;
        case 82: _t->onRadioButton_23Pressed(); break;
        case 83: _t->onRadioButton_24Pressed(); break;
        case 84: _t->onRadioButton_25Pressed(); break;
        case 85: _t->onRadioButton_26Pressed(); break;
        case 86: _t->onRadioButton_27Pressed(); break;
        case 87: _t->onRadioButton_28Pressed(); break;
        case 88: _t->onRadioButton_31Pressed(); break;
        case 89: _t->onRadioButton_32Pressed(); break;
        case 90: _t->onRadioButton_33Pressed(); break;
        case 91: _t->onRadioButton_34Pressed(); break;
        case 92: _t->onRadioButton_37Pressed(); break;
        case 93: _t->onRadioButton_38Pressed(); break;
        case 94: _t->onRadioButton_41Pressed(); break;
        case 95: _t->onRadioButton_42Pressed(); break;
        case 96: _t->onRadioButton_43Pressed(); break;
        case 97: _t->onRadioButton_44Pressed(); break;
        case 98: _t->onRadioButton_45Pressed(); break;
        case 99: _t->onRadioButton_46Pressed(); break;
        case 100: _t->onRadioButton_49Pressed(); break;
        case 101: _t->onRadioButton_50Pressed(); break;
        case 102: _t->onRadioButton_51Pressed(); break;
        case 103: _t->onRadioButton_52Pressed(); break;
        case 104: _t->onRadioButton_55Pressed(); break;
        case 105: _t->onRadioButton_56Pressed(); break;
        case 106: _t->onRadioButton_59Pressed(); break;
        case 107: _t->onRadioButton_60Pressed(); break;
        case 108: _t->onRadioButton_61Pressed(); break;
        case 109: _t->onRadioButton_62Pressed(); break;
        case 110: _t->onRadioButton_63Pressed(); break;
        case 111: _t->onRadioButton_64Pressed(); break;
        case 112: _t->onRadioButton_67Pressed(); break;
        case 113: _t->onRadioButton_68Pressed(); break;
        case 114: _t->onRadioButton_69Pressed(); break;
        case 115: _t->onRadioButton_70Pressed(); break;
        case 116: _t->onRadioButton_73Pressed(); break;
        case 117: _t->onRadioButton_74Pressed(); break;
        case 118: _t->onRadioButton_77Pressed(); break;
        case 119: _t->onRadioButton_78Pressed(); break;
        case 120: _t->onRadioButton_79Pressed(); break;
        case 121: _t->onRadioButton_80Pressed(); break;
        case 122: _t->onRadioButton_81Pressed(); break;
        case 123: _t->onRadioButton_82Pressed(); break;
        case 124: _t->onRadioButton_85Pressed(); break;
        case 125: _t->onRadioButton_86Pressed(); break;
        case 126: _t->onRadioButton_87Pressed(); break;
        case 127: _t->onRadioButton_88Pressed(); break;
        case 128: _t->onRadioButton_91Pressed(); break;
        case 129: _t->onRadioButton_92Pressed(); break;
        case 130: _t->onRadioButton_95Pressed(); break;
        case 131: _t->onRadioButton_96Pressed(); break;
        case 132: _t->onRadioButton_97Pressed(); break;
        case 133: _t->onRadioButton_98Pressed(); break;
        case 134: _t->onRadioButton_99Pressed(); break;
        case 135: _t->onRadioButton_100Pressed(); break;
        case 136: _t->onRadioButton_103Pressed(); break;
        case 137: _t->onRadioButton_104Pressed(); break;
        case 138: _t->onRadioButton_105Pressed(); break;
        case 139: _t->onRadioButton_106Pressed(); break;
        case 140: _t->updateGraph(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::connectionError)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 141)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 141;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 141)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 141;
    }
    return _id;
}

// SIGNAL 0
void MainWindow::connectionError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
