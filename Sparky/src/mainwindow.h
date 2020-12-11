#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QProgressDialog>
#include "modbus.h"
#include "ui_about.h"
#include "modbus-rtu.h"
#include "modbus.h"
#include "qcgaugewidget.h"

#define RAZ_REG_WATERCUT 
#define RAZ_REG_WATERCUT 
#define MAX_PIPE 18
#define RAZ true
#define EEA false

/// calibration file names

#define HIGH                        "G:\\HIGHCUT\\HC"
#define FULL                        "G:\\FULLCUT\\FC" 
#define MID                         "G:\\MIDCUT\\MC"
#define LOW                         "G:\\LOWCUT\\LC"

#define FILE_LIST_LC               "Filelist.LST"
#define AMB_TWENTY_LC              "AMB_020.LCT"
#define TWENTY_FIFTYFIVE_LC        "020_055.LCT"
#define FIFTYFIVE_THIRTYEIGHT_LC   "055_038.LCI"
#define CALIBRAT_LC                "CALIBRAT.LCI"
#define ADJUSTED_LC                "ADJUSTED.LCI"
#define ROLLOVER_LC                "ROLLOVER.LCR"

#define FILE_LIST_MC               "Filelist.LST"
#define AMB_TWENTY_MC              "AMB_020.MCT"
#define TWENTY_FIFTYFIVE_MC        "020_055.MCT"
#define FIFTYFIVE_THIRTYEIGHT_MC   "055_038.MCI"
#define CALIBRAT_MC                "CALIBRAT.MCI"
#define ADJUSTED_MC                "ADJUSTED.MCI"
#define ROLLOVER_MC                "ROLLOVER.MCR"

#define FILE_LIST_FC               "Filelist.LST"
#define AMB_TWENTY_FC              "AMB_020.FCT"
#define TWENTY_FIFTYFIVE_FC        "020_055.FCT"
#define FIFTYFIVE_THIRTYEIGHT_FC   "055_038.FCI"
#define CALIBRAT_FC                "CALIBRAT.FCI"
#define ADJUSTED_FC                "ADJUSTED.FCI"
#define ROLLOVER_FC                "ROLLOVER.FCR"

#define FILE_LIST_HC               "Filelist.LST"
#define AMB_TWENTY_HC              "AMB_020.HCT"
#define TWENTY_FIFTYFIVE_HC        "020_055.HCT"
#define FIFTYFIVE_THIRTYEIGHT_HC   "055_038.HCI"
#define CALIBRAT_HC                "CALIBRAT.HCI"
#define ADJUSTED_HC                "ADJUSTED.HCI"
#define ROLLOVER_HC                "ROLLOVER.HCR"

QT_CHARTS_USE_NAMESPACE

class AboutDialog : public QDialog, public Ui::AboutDialog
{
public:
    AboutDialog( QWidget * _parent ) :
        QDialog( _parent )
    {
        setupUi( this );
        aboutTextLabel->setText(aboutTextLabel->text().arg( "0.3.0" ) );
    }
};

typedef struct pipe_object
{
    int serialNumber;
    int loopVolume;
    float smallPumpInjectionRate;
    float bigPumpInjectionRate;
    float calibrationLimit;

} PIPE;

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget * parent = 0 );
    ~MainWindow();

    void delay(int);


    modbus_t * m_serialModbus;
    modbus_t * m_serialModbus_2;
    modbus_t * m_serialModbus_3;
    modbus_t * m_serialModbus_4;
    modbus_t * m_serialModbus_5;
    modbus_t * m_serialModbus_6;

    modbus_t*  modbus() { return m_serialModbus; }
    modbus_t*  modbus_2() { return m_serialModbus_2; }
    modbus_t*  modbus_3() { return m_serialModbus_3; }
    modbus_t*  modbus_4() { return m_serialModbus_4; }
    modbus_t*  modbus_5() { return m_serialModbus_5; }
    modbus_t*  modbus_6() { return m_serialModbus_6; }

    int setupModbusPort();
    int setupModbusPort_2();
    int setupModbusPort_3();
    int setupModbusPort_4();
    int setupModbusPort_5();
    int setupModbusPort_6();

    void changeModbusInterface(const QString &port, char parity);
    void changeModbusInterface_2(const QString &port, char parity);
    void changeModbusInterface_3(const QString &port, char parity);
    void changeModbusInterface_4(const QString &port, char parity);
    void changeModbusInterface_5(const QString &port, char parity);
    void changeModbusInterface_6(const QString &port, char parity);

    void releaseSerialModbus();
    void releaseSerialModbus_2();
    void releaseSerialModbus_3();
    void releaseSerialModbus_4();
    void releaseSerialModbus_5();
    void releaseSerialModbus_6();

    void busMonitorAddItem( bool isRequest,uint16_t slave,uint8_t func,uint16_t addr,uint16_t nb,uint16_t expectedCRC,uint16_t actualCRC );
    static void stBusMonitorAddItem( modbus_t * modbus,uint8_t isOut, uint16_t slave, uint8_t func, uint16_t addr,uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC );
    static void stBusMonitorRawData( modbus_t * modbus, uint8_t * data,uint8_t dataLen, uint8_t addNewline );
    void busMonitorRawData( uint8_t * data, uint8_t dataLen, bool addNewline );
    void connectRadioButtons();
    void connectSerialPort();
    void connectActions();
    void connectModbusMonitor();
    void connectTimers();
    void connectRegisters();
    void connectLoopDependentData();
    void connectCalibrationControls();
    void connectProfiler();
    void connectToolbar();
    void initializeGauges();
    void setupModbusPorts();
    void updateTabIcon(int, bool);
    void updateChartTitle();
    void initializeTabIcons();
    float toFloat(QByteArray arr);
    void initializeModbusMonitor();
    void onFunctionCodeChanges();
    QString sendCalibrationRequest(int, modbus_t *, int, int, int, int, uint8_t *, uint16_t *, bool, bool, QString);

private slots:

    void createLoopFiles(const int, const QString, QFile &, QFile &, QFile &, QFile &, QFile &, QFile &, QFile &);

    void onRtuPortActive(bool);
    void onRtuPortActive_2(bool);
    void onRtuPortActive_3(bool);
    void onRtuPortActive_4(bool);
    void onRtuPortActive_5(bool);
    void onRtuPortActive_6(bool);

    void changeSerialPort(int);
    void changeSerialPort_2(int);
    void changeSerialPort_3(int);
    void changeSerialPort_4(int);
    void changeSerialPort_5(int);
    void changeSerialPort_6(int);

    void calibration_L1P1();
    void calibration_L1P2();
    void calibration_L1P3();
    void calibration_L2P1();
    void calibration_L2P2();
    void calibration_L2P3();
    void calibration_L3P1();
    void calibration_L3P2();
    void calibration_L3P3();
    void calibration_L4P1();
    void calibration_L4P2();
    void calibration_L4P3();
    void calibration_L5P1();
    void calibration_L5P2();
    void calibration_L5P3();
    void calibration_L6P1();
    void calibration_L6P2();
    void calibration_L6P3();

    void initializeToolbarIcons(void);
    void initializeFrequencyGauge();
    void initializeTemperatureGauge();
    void initializeDensityGauge();
    void initializeRPGauge();
    void updateFrequencyGauge();    
    void updateTemperatureGauge();    
    void updateDensityGauge();    
    void updateRPGauge();
    void onLoopTabChanged(int);
    void clearMonitors( void );
    void updateRequestPreview( void );
    void updateRegisterView( void );
    void enableHexView( void );
    void sendModbusRequest( void );
    void onSendButtonPress( void );
    void pollForDataOnBus( void );
    void openBatchProcessor();
    void aboutQModBus( void );
    void onCheckBoxChecked(bool);
    void resetStatus( void );
    void setStatusError(const QString &msg);    
    void onFloatButtonPressed(bool);
    void onIntegerButtonPressed(bool);
    void onCoilButtonPressed(bool);
    void onReadButtonPressed(bool);
    void onWriteButtonPressed(bool);
    void onEquationButtonPressed();
    void loadCsvFile();
    void loadCsvTemplate();
    void onUploadEquation();
    void onDownloadEquation();
    void updateRegisters(const bool, const int);
    void onProductBtnPressed();
    void onDownloadButtonChecked(bool);
    void saveCsvFile();
    void onEquationTableChecked(bool);
    void onUnlockFactoryDefaultBtnPressed();
    void onLockFactoryDefaultBtnPressed();
    void onUpdateFactoryDefaultPressed();

    // radio buttons
    void onRadioButtonPressed();
    void onRadioButton_2Pressed();
    void onRadioButton_3Pressed();
    void onRadioButton_4Pressed();
    void onRadioButton_7Pressed();
    void onRadioButton_8Pressed();
    void onRadioButton_9Pressed();
    void onRadioButton_10Pressed();
    void onRadioButton_15Pressed();
    void onRadioButton_16Pressed();
    void onRadioButton_19Pressed();
    void onRadioButton_20Pressed();
    void onRadioButton_23Pressed();
    void onRadioButton_24Pressed();
    void onRadioButton_25Pressed();
    void onRadioButton_26Pressed();
    void onRadioButton_27Pressed();
    void onRadioButton_28Pressed();
    void onRadioButton_31Pressed();
    void onRadioButton_32Pressed();
    void onRadioButton_33Pressed();
    void onRadioButton_34Pressed();
    void onRadioButton_37Pressed();
    void onRadioButton_38Pressed();
    void onRadioButton_41Pressed();
    void onRadioButton_42Pressed();
    void onRadioButton_43Pressed();
    void onRadioButton_44Pressed();
    void onRadioButton_45Pressed();
    void onRadioButton_46Pressed();
    void onRadioButton_49Pressed();
    void onRadioButton_50Pressed();
    void onRadioButton_51Pressed();
    void onRadioButton_52Pressed();
    void onRadioButton_55Pressed();
    void onRadioButton_56Pressed();
    void onRadioButton_59Pressed();
    void onRadioButton_60Pressed();
    void onRadioButton_61Pressed();
    void onRadioButton_62Pressed();
    void onRadioButton_63Pressed();
    void onRadioButton_64Pressed();
    void onRadioButton_67Pressed();
    void onRadioButton_68Pressed();
    void onRadioButton_69Pressed();
    void onRadioButton_70Pressed();
    void onRadioButton_73Pressed();
    void onRadioButton_74Pressed();
    void onRadioButton_77Pressed();
    void onRadioButton_78Pressed();
    void onRadioButton_79Pressed();
    void onRadioButton_80Pressed();
    void onRadioButton_81Pressed();
    void onRadioButton_82Pressed();
    void onRadioButton_85Pressed();
    void onRadioButton_86Pressed();
    void onRadioButton_87Pressed();
    void onRadioButton_88Pressed();
    void onRadioButton_91Pressed();
    void onRadioButton_92Pressed();
    void onRadioButton_95Pressed();
    void onRadioButton_96Pressed();
    void onRadioButton_97Pressed();
    void onRadioButton_98Pressed();
    void onRadioButton_99Pressed();
    void onRadioButton_100Pressed();
    void onRadioButton_105Pressed();
    void onRadioButton_106Pressed();
    void onRadioButton_194Pressed();
    void onRadioButton_195Pressed();
    void onRadioButton_196Pressed();
    void onRadioButton_197Pressed();
    void onRadioButton_200Pressed();
    void onRadioButton_201Pressed();
    void onRadioButton_202Pressed();
    void onRadioButton_203Pressed();
    void onRadioButton_204Pressed();
    void onRadioButton_205Pressed();
    void onRadioButton_206Pressed();
    void onRadioButton_207Pressed();
    void onRadioButton_208Pressed();
    void onRadioButton_209Pressed();
    void onRadioButton_210Pressed();
    void onRadioButton_211Pressed();
    void onRadioButton_212Pressed();
    void onRadioButton_213Pressed();
    void onRadioButton_214Pressed();
    void onRadioButton_215Pressed();
    void onRadioButton_216Pressed();
    void onRadioButton_217Pressed();
    void onRadioButton_218Pressed();
    void onRadioButton_219Pressed();
    void onRadioButton_220Pressed();
    void onRadioButton_221Pressed();
    void onRadioButton_222Pressed();
    void onRadioButton_223Pressed();
    void onRadioButton_224Pressed();
    void onRadioButton_225Pressed();
    void onRadioButton_226Pressed();
    void onRadioButton_227Pressed();
    void onRadioButton_228Pressed();
    void onRadioButton_229Pressed();
    void onRadioButton_230Pressed();
    void onRadioButton_231Pressed();
    void onRadioButton_234Pressed();
    void onRadioButton_235Pressed();
    void onRadioButton_236Pressed();
    void onRadioButton_237Pressed();


    // update graph
    void updateGraph();


signals:
    void connectionError(const QString &msg);

private:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    Ui::MainWindowClass * ui;

    /// loop files
    QFile file1_L1P1;
    QFile file2_L1P1;
    QFile file3_L1P1;
    QFile file4_L1P1;
    QFile file5_L1P1;
    QFile file6_L1P1;
    QFile file7_L1P1;
   
    QFile file1_L1P2;
    QFile file2_L1P2;
    QFile file3_L1P2;
    QFile file4_L1P2;
    QFile file5_L1P2;
    QFile file6_L1P2;
    QFile file7_L1P2;

    QFile file1_L1P3;
    QFile file2_L1P3;
    QFile file3_L1P3;
    QFile file4_L1P3;
    QFile file5_L1P3;
    QFile file6_L1P3;
    QFile file7_L1P3;
   
    QFile file1_L2P1;
    QFile file2_L2P1;
    QFile file3_L2P1;
    QFile file4_L2P1;
    QFile file5_L2P1;
    QFile file6_L2P1;
    QFile file7_L2P1;

    QFile file1_L2P2;
    QFile file2_L2P2;
    QFile file3_L2P2;
    QFile file4_L2P2;
    QFile file5_L2P2;
    QFile file6_L2P2;
    QFile file7_L2P2;
   
    QFile file1_L2P3;
    QFile file2_L2P3;
    QFile file3_L2P3;
    QFile file4_L2P3;
    QFile file5_L2P3;
    QFile file6_L2P3;
    QFile file7_L2P3;

    QFile file1_L3P1;
    QFile file2_L3P1;
    QFile file3_L3P1;
    QFile file4_L3P1;
    QFile file5_L3P1;
    QFile file6_L3P1;
    QFile file7_L3P1;
   
    QFile file1_L3P2;
    QFile file2_L3P2;
    QFile file3_L3P2;
    QFile file4_L3P2;
    QFile file5_L3P2;
    QFile file6_L3P2;
    QFile file7_L3P2;

    QFile file1_L3P3;
    QFile file2_L3P3;
    QFile file3_L3P3;
    QFile file4_L3P3;
    QFile file5_L3P3;
    QFile file6_L3P3;
    QFile file7_L3P3;
   
    QFile file1_L4P1;
    QFile file2_L4P1;
    QFile file3_L4P1;
    QFile file4_L4P1;
    QFile file5_L4P1;
    QFile file6_L4P1;
    QFile file7_L4P1;

    QFile file1_L4P2;
    QFile file2_L4P2;
    QFile file3_L4P2;
    QFile file4_L4P2;
    QFile file5_L4P2;
    QFile file6_L4P2;
    QFile file7_L4P2;
   
    QFile file1_L4P3;
    QFile file2_L4P3;
    QFile file3_L4P3;
    QFile file4_L4P3;
    QFile file5_L4P3;
    QFile file6_L4P3;
    QFile file7_L4P3;

    QFile file1_L5P1;
    QFile file2_L5P1;
    QFile file3_L5P1;
    QFile file4_L5P1;
    QFile file5_L5P1;
    QFile file6_L5P1;
    QFile file7_L5P1;
   
    QFile file1_L5P2;
    QFile file2_L5P2;
    QFile file3_L5P2;
    QFile file4_L5P2;
    QFile file5_L5P2;
    QFile file6_L5P2;
    QFile file7_L5P2;

    QFile file1_L5P3;
    QFile file2_L5P3;
    QFile file3_L5P3;
    QFile file4_L5P3;
    QFile file5_L5P3;
    QFile file6_L5P3;
    QFile file7_L5P3;
   
    QFile file1_L6P1;
    QFile file2_L6P1;
    QFile file3_L6P1;
    QFile file4_L6P1;
    QFile file5_L6P1;
    QFile file6_L6P1;
    QFile file7_L6P1;
 
    QFile file1_L6P2;
    QFile file2_L6P2;
    QFile file3_L6P2;
    QFile file4_L6P2;
    QFile file5_L6P2;
    QFile file6_L6P2;
    QFile file7_L6P2;

    QFile file1_L6P3;
    QFile file2_L6P3;
    QFile file3_L6P3;
    QFile file4_L6P3;
    QFile file5_L6P3;
    QFile file6_L6P3;
    QFile file7_L6P3;

    modbus_t * m_modbus;
    modbus_t * m_modbus_2;
    modbus_t * m_modbus_3;
    modbus_t * m_modbus_4;
    modbus_t * m_modbus_5;
    modbus_t * m_modbus_6;
    modbus_t * m_modbus_snipping;

    QWidget * m_statusInd;
    QLabel * m_statusText;
    QTimer * m_pollTimer;
    QTimer * m_statusTimer;

    bool m_tcpActive;
    bool m_poll;

    // 3 axis line graph display
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QValueAxis *axisY3;
    QSplineSeries *series;
    QChartView *chartView;

    //
    // gauge display
    //
    QcGaugeWidget * m_frequencyGauge;
    QcNeedleItem * m_frequencyNeedle;
    QcGaugeWidget * m_temperatureGauge;
    QcNeedleItem * m_temperatureNeedle;
    QcGaugeWidget * m_densityGauge;
    QcNeedleItem * m_densityNeedle;
    QcGaugeWidget * m_RPGauge;
    QcNeedleItem * m_RPNeedle;
    
    int REG_SN_PIPE[MAX_PIPE];
    int REG_WATERCUT[MAX_PIPE];
    int REG_TEMPERATURE[MAX_PIPE];
    int REG_EMULSTION_PHASE[MAX_PIPE];
    int REG_SALINITY[MAX_PIPE];
    int REG_HARDWARE_VERSION[MAX_PIPE];
    int REG_FIRMWARE_VERSION[MAX_PIPE];
    int REG_OIL_ADJUST[MAX_PIPE];
    int REG_WATER_ADJUST[MAX_PIPE];
    int REG_FREQ[MAX_PIPE];
    int REG_FREQ_AVG[MAX_PIPE];
    int REG_WATERCUT_AVG[MAX_PIPE];
    int REG_WATERCUT_RAW[MAX_PIPE];
    int REG_ANALYZER_MODE[MAX_PIPE];
    int REG_TEMP_AVG[MAX_PIPE];
    int REG_TEMP_ADJUST[MAX_PIPE];
    int REG_TEMP_USER[MAX_PIPE];
    int REG_PROC_AVGING[MAX_PIPE];
    int REG_OIL_INDEX[MAX_PIPE];
    int REG_OIL_P0[MAX_PIPE];
    int REG_OIL_P1[MAX_PIPE];
    int REG_OIL_FREQ_LOW[MAX_PIPE];
    int REG_OIL_FREQ_HIGH[MAX_PIPE];
    int REG_SAMPLE_PERIOD[MAX_PIPE];
    int REG_AO_LRV[MAX_PIPE];
    int REG_AO_URV[MAX_PIPE];
    int REG_AO_DAMPEN[MAX_PIPE];
    int REG_BAUD_RATE[MAX_PIPE];
    int REG_SLAVE_ADDRESS[MAX_PIPE];
    int REG_STOP_BITS[MAX_PIPE];
    int REG_OIL_RP[MAX_PIPE];
    int REG_WATER_RP[MAX_PIPE];
    int REG_DENSITY_MODE[MAX_PIPE];
    int REG_OIL_CALC_MAX[MAX_PIPE];
    int REG_OIL_PHASE_CUTOFF[MAX_PIPE];
    int REG_TEMP_OIL_NUM_CURVES[MAX_PIPE];
    int REG_STREAM[MAX_PIPE];
    int REG_OIL_RP_AVG[MAX_PIPE];
    int REG_PLACE_HOLDER[MAX_PIPE];
    int REG_OIL_SAMPLE[MAX_PIPE];
    int REG_RTC_SEC[MAX_PIPE];
    int REG_RTC_MIN[MAX_PIPE];
    int REG_RTC_HR[MAX_PIPE];
    int REG_RTC_DAY[MAX_PIPE];
    int REG_RTC_MON[MAX_PIPE];
    int REG_RTC_YR[MAX_PIPE];
    int REG_RTC_SEC_IN[MAX_PIPE];
    int REG_RTC_MIN_IN[MAX_PIPE];
    int REG_RTC_HR_IN[MAX_PIPE];
    int REG_RTC_DAY_IN[MAX_PIPE];
    int REG_RTC_MON_IN[MAX_PIPE];
    int REG_RTC_YR_IN[MAX_PIPE];
    int REG_AO_MANUAL_VAL[MAX_PIPE];
    int REG_AO_TRIMLO[MAX_PIPE];
    int REG_AO_TRIMHI[MAX_PIPE];
    int REG_DENSITY_ADJ[MAX_PIPE];
    int REG_DENSITY_UNIT[MAX_PIPE];
    int REG_WC_ADJ_DENS[MAX_PIPE];
    int REG_DENSITY_D3[MAX_PIPE];
    int REG_DENSITY_D2[MAX_PIPE];
    int REG_DENSITY_D1[MAX_PIPE];
    int REG_DENSITY_D0[MAX_PIPE];
    int REG_DENSITY_CAL_VAL[MAX_PIPE];
    int REG_MODEL_CODE_0[MAX_PIPE];
    int REG_MODEL_CODE_1[MAX_PIPE];
    int REG_MODEL_CODE_2[MAX_PIPE];
    int REG_MODEL_CODE_3[MAX_PIPE];
    int REG_LOGGING_PERIOD[MAX_PIPE];
    int REG_PASSWORD[MAX_PIPE];
    int REG_STATISTICS[MAX_PIPE];
    int REG_ACTIVE_ERROR[MAX_PIPE];
    int REG_AO_ALARM_MODE[MAX_PIPE];
    int REG_AO_OUTPUT[MAX_PIPE];
    int REG_PHASE_HOLD_CYCLES[MAX_PIPE];
    int REG_RELAY_DELAY[MAX_PIPE];
    int REG_RELAY_SETPOINT[MAX_PIPE];
    int REG_AO_MODE[MAX_PIPE];
    int REG_OIL_DENSITY[MAX_PIPE];
    int REG_OIL_DENSITY_MODBUS[MAX_PIPE];
    int REG_OIL_DENSITY_AI[MAX_PIPE];
    int REG_OIL_DENSITY_MANUAL[MAX_PIPE];
    int REG_OIL_DENSITY_AI_LRV[MAX_PIPE];
    int REG_OIL_DENSITY_AI_URV[MAX_PIPE];
    int REG_OIL_DENS_CORR_MODE[MAX_PIPE];
    int REG_AI_TRIMLO[MAX_PIPE];
    int REG_AI_TRIMHI[MAX_PIPE];
    int REG_AI_MEASURE[MAX_PIPE];
    int REG_AI_TRIMMED[MAX_PIPE];

	bool isModbusTransmissionFailed;
};

#endif // MAINWINDOW_H
