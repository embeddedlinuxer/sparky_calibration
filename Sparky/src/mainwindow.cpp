#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QScrollBar>
#include <QTime>
#include <QFileDialog>
#include <QThread>
#include <errno.h>
#include <QListWidget>
#include <QProgressDialog>
#include "mainwindow.h"
#include "BatchProcessor.h"
#include "modbus.h"
#include "modbus-private.h"
#include "modbus-rtu.h"
#include "ui_mainwindow.h"
#include "qextserialenumerator.h"

#define SLAVE_CALIBRATION   0xFA
#define FUNC_READ_FLOAT     0x04
#define FUNC_READ_INT       0x04 
#define FUNC_READ_COIL      0x01 
#define FUNC_WRITE_FLOAT    0x10
#define FUNC_WRITE_INT      0x06
#define FUNC_WRITE_COIL     0x05
#define BYTE_READ_FLOAT     2
#define BYTE_READ_INT       1
#define BYTE_READ_COIL      1
#define FLOAT_R             0
#define FLOAT_W             1
#define INT_R               2
#define INT_W               3
#define COIL_R              4
#define COIL_W              5


QT_CHARTS_USE_NAMESPACE

const int DataTypeColumn = 0;
const int AddrColumn = 1;
const int DataColumn = 2;

extern MainWindow * globalMainWin;

MainWindow::MainWindow( QWidget * _parent ) :
	QMainWindow( _parent ),
	ui( new Ui::MainWindowClass ),
	m_modbus( NULL ),
    m_modbus_2( NULL ),
    m_modbus_3( NULL ),
    m_modbus_4( NULL ),
    m_modbus_5( NULL ),
    m_modbus_6( NULL ),
    m_modbus_snipping( NULL ),
    m_serialModbus( NULL ),
    m_serialModbus_2( NULL ),
    m_serialModbus_3( NULL ),
    m_serialModbus_4( NULL ),
    m_serialModbus_5( NULL ),
    m_serialModbus_6( NULL ),
	m_poll(false),
	isModbusTransmissionFailed(false)
{
	ui->setupUi(this);

    /// versioning
    setWindowTitle(SPARKY);

    updateRegisters(EEA,0); // L1-P1-EEA
    initializeToolbarIcons();
    initializeGauges();
    initializeTabIcons();
    updateRegisterView();
    updateRequestPreview();
    enableHexView();
    setupModbusPorts();
    onLoopTabChanged(0);
    updateGraph();
    initializeModbusMonitor();

    ui->regTable->setColumnWidth( 0, 150 );
    m_statusInd = new QWidget;
    m_statusInd->setFixedSize( 16, 16 );
    m_statusText = new QLabel;
    ui->statusBar->addWidget( m_statusInd );
    ui->statusBar->addWidget( m_statusText, 10 );
    resetStatus();

    /// connections 
    connectLoopDependentData();
    connectRadioButtons();
    connectSerialPort();
    connectActions();
    connectModbusMonitor();
    connectTimers();
    connectCalibrationControls();
    connectProfiler();
    connectToolbar();

    /// clear connection at start
    updateTabIcon(0, false);
}


MainWindow::~MainWindow()
{
    releaseSerialModbus();
    releaseSerialModbus_2();
    releaseSerialModbus_3();
    releaseSerialModbus_4();
    releaseSerialModbus_5();
    releaseSerialModbus_6();

	delete ui;
}

void
MainWindow::
delay(int sec = 2)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}


void
MainWindow::
initializeModbusMonitor()
{
    ui->groupBox_103->setEnabled(TRUE);
    ui->groupBox_106->setEnabled(FALSE);
    ui->groupBox_107->setEnabled(FALSE);
    ui->functionCode->setCurrentIndex(3);
}


void
MainWindow::
initializeToolbarIcons() {

    //ui->toolBar->addAction(ui->actionConnect);
    //ui->toolBar->addAction(ui->actionDisconnect);
    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addAction(ui->actionSave);
    ui->actionDisconnect->setDisabled(TRUE);
    ui->actionConnect->setEnabled(TRUE);
}

void
MainWindow::
keyPressEvent(QKeyEvent* event)
{
	if( event->key() == Qt::Key_Control )
	{
		//set flag to request polling
        if( m_modbus != NULL )	m_poll = true;
        if( ! m_pollTimer->isActive() )	ui->sendBtn->setText( tr("Poll") );
	}
}

void
MainWindow::
keyReleaseEvent(QKeyEvent* event)
{
	if( event->key() == Qt::Key_Control )
	{
		m_poll = false;
        if( ! m_pollTimer->isActive() )	ui->sendBtn->setText( tr("Send") );
	}
}

void MainWindow::onSendButtonPress( void )
{
	// if already polling then stop
	if( m_pollTimer->isActive() )
	{
		m_pollTimer->stop();
		ui->sendBtn->setText( tr("Send") );
	}
	else
	{
		// if polling requested then enable timer
		if( m_poll )
		{
			m_pollTimer->start( 1000 );
			ui->sendBtn->setText( tr("Stop") );
		}

		sendModbusRequest();
	}
}

void MainWindow::busMonitorAddItem( bool isRequest,
                    uint16_t slave,
					uint8_t func,
					uint16_t addr,
					uint16_t nb,
					uint16_t expectedCRC,
					uint16_t actualCRC )
{
	QTableWidget * bm = ui->busMonTable;
	const int rowCount = bm->rowCount();
	bm->setRowCount( rowCount+1 );

    QTableWidgetItem * numItem;
	QTableWidgetItem * ioItem = new QTableWidgetItem( isRequest ? tr( "Req >>" ) : tr( "<< Resp" ) );
	QTableWidgetItem * slaveItem = new QTableWidgetItem( QString::number( slave ) );
	QTableWidgetItem * funcItem = new QTableWidgetItem( QString::number( func ) );
	QTableWidgetItem * addrItem = new QTableWidgetItem( QString::number( addr ) );
    (ui->radioButton_181->isChecked()) ?numItem  = new QTableWidgetItem( QString::number( 2 ) ) : numItem = new QTableWidgetItem( QString::number( 1 ) );
	QTableWidgetItem * crcItem = new QTableWidgetItem;

	if( func > 127 )
	{
		addrItem->setText( QString() );
		numItem->setText( QString() );
		funcItem->setText( tr( "Exception (%1)" ).arg( func-128 ) );
		funcItem->setForeground( Qt::red );
	}
	else
	{
		if( expectedCRC == actualCRC )
		{
			crcItem->setText( QString().sprintf( "%.4x", actualCRC ) );
		}
		else
		{
			crcItem->setText( QString().sprintf( "%.4x (%.4x)", actualCRC, expectedCRC ) );
			crcItem->setForeground( Qt::red );
		}
	}
	ioItem->setFlags( ioItem->flags() & ~Qt::ItemIsEditable );
	slaveItem->setFlags( slaveItem->flags() & ~Qt::ItemIsEditable );
	funcItem->setFlags( funcItem->flags() & ~Qt::ItemIsEditable );
	addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
	numItem->setFlags( numItem->flags() & ~Qt::ItemIsEditable );
	crcItem->setFlags( crcItem->flags() & ~Qt::ItemIsEditable );
	bm->setItem( rowCount, 0, ioItem );
	bm->setItem( rowCount, 1, slaveItem );
    bm->setItem( rowCount, 2, funcItem );
	bm->setItem( rowCount, 3, addrItem );
	bm->setItem( rowCount, 4, numItem );
	bm->setItem( rowCount, 5, crcItem );
	bm->verticalScrollBar()->setValue( bm->verticalScrollBar()->maximum() );
}


void MainWindow::busMonitorRawData( uint8_t * data, uint8_t dataLen, bool addNewline )
{
	if( dataLen > 0 )
	{
		QString dump = ui->rawData->toPlainText();
		for( int i = 0; i < dataLen; ++i )
		{
			dump += QString().sprintf( "%.2x ", data[i] );
		}
		if( addNewline )
		{
			dump += "\n";
		}
		ui->rawData->setPlainText( dump );
		ui->rawData->verticalScrollBar()->setValue( 100000 );
		ui->rawData->setLineWrapMode( QPlainTextEdit::NoWrap );
	}
}

// static
void MainWindow::stBusMonitorAddItem( modbus_t * modbus, uint8_t isRequest, uint16_t slave, uint8_t func, uint16_t addr, uint16_t nb, uint16_t expectedCRC, uint16_t actualCRC )
{
    Q_UNUSED(modbus);
    globalMainWin->busMonitorAddItem( isRequest, slave, func, addr+1, nb, expectedCRC, actualCRC );
}

// static
void MainWindow::stBusMonitorRawData( modbus_t * modbus, uint8_t * data, uint8_t dataLen, uint8_t addNewline )
{
    Q_UNUSED(modbus);
    globalMainWin->busMonitorRawData( data, dataLen, addNewline != 0 );
}

static QString descriptiveDataTypeName( int funcCode )
{
	switch( funcCode )
	{
		case MODBUS_FC_READ_COILS:
		case MODBUS_FC_WRITE_SINGLE_COIL:
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
			return "Coil (binary)";
		case MODBUS_FC_READ_DISCRETE_INPUTS:
			return "Discrete Input (binary)";
		case MODBUS_FC_READ_HOLDING_REGISTERS:
		case MODBUS_FC_WRITE_SINGLE_REGISTER:
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
			return "Holding Register (16 bit)";
		case MODBUS_FC_READ_INPUT_REGISTERS:
			return "Input Register (16 bit)";
		default:
			break;
	}
	return "Unknown";
}


static inline QString embracedString( const QString & s )
{
	return s.section( '(', 1 ).section( ')', 0, 0 );
}


static inline int stringToHex( QString s )
{
	return s.replace( "0x", "" ).toInt( NULL, 16 );
}


void MainWindow::updateRequestPreview( void )
{
	const int slave = ui->slaveID->value();
    const int func = stringToHex( embracedString(ui->functionCode->currentText() ) );
	const int addr = ui->startAddr->value()-1;
	const int num = ui->numCoils->value();
	if( func == MODBUS_FC_WRITE_SINGLE_COIL || func == MODBUS_FC_WRITE_SINGLE_REGISTER )
	{
		ui->requestPreview->setText(
			QString().sprintf( "%.2x  %.2x  %.2x %.2x ",
					slave,
					func,
					addr >> 8,
					addr & 0xff ) );
	}
	else
	{
		ui->requestPreview->setText(
			QString().sprintf( "%.2x  %.2x  %.2x %.2x  %.2x %.2x",
					slave,
					func,
					addr >> 8,
					addr & 0xff,
					num >> 8,
					num & 0xff ) );
	}
}




void MainWindow::updateRegisterView( void )
{
	const int func = stringToHex( embracedString(ui->functionCode->currentText() ) );
	const QString funcType = descriptiveDataTypeName( func );
	const int addr = ui->startAddr->value();

	int rowCount = 0;
	switch( func )
	{
		case MODBUS_FC_WRITE_SINGLE_REGISTER:
		case MODBUS_FC_WRITE_SINGLE_COIL:
			ui->numCoils->setEnabled( false );
			rowCount = 1;
			break;
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
			rowCount = ui->numCoils->value();
		default:
			ui->numCoils->setEnabled( true );
			break;
	}

	ui->regTable->setRowCount( rowCount );
	for( int i = 0; i < rowCount; ++i )
	{
		QTableWidgetItem * dtItem = new QTableWidgetItem( funcType );
		QTableWidgetItem * addrItem = new QTableWidgetItem( QString::number( addr+i ) );
		QTableWidgetItem * dataItem = new QTableWidgetItem( QString::number( 0 ) );

		dtItem->setFlags( dtItem->flags() & ~Qt::ItemIsEditable	);
		addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
		ui->regTable->setItem( i, DataTypeColumn, dtItem );
		ui->regTable->setItem( i, AddrColumn, addrItem );
		ui->regTable->setItem( i, DataColumn, dataItem );
	}

	ui->regTable->setColumnWidth( 0, 150 );
}


void MainWindow::enableHexView( void )
{
	const int func = stringToHex( embracedString(
					ui->functionCode->currentText() ) );

	bool b_enabled =
		func == MODBUS_FC_READ_HOLDING_REGISTERS ||
		func == MODBUS_FC_READ_INPUT_REGISTERS;

	ui->checkBoxHexData->setEnabled( b_enabled );
}


void MainWindow::sendModbusRequest( void )
{
    // UPDATE m_modbus_snipping WITH THE CURRENT
    if (ui->tabWidget_2->currentIndex() == 0)      m_modbus_snipping = m_modbus;
    else if (ui->tabWidget_2->currentIndex() == 1) m_modbus_snipping = m_modbus_2;
    else if (ui->tabWidget_2->currentIndex() == 2) m_modbus_snipping = m_modbus_3;
    else if (ui->tabWidget_2->currentIndex() == 3) m_modbus_snipping = m_modbus_4;
    else if (ui->tabWidget_2->currentIndex() == 4) m_modbus_snipping = m_modbus_5;
    else                                           m_modbus_snipping = m_modbus_6;

	if( m_modbus_snipping == NULL )
	{
		setStatusError( tr("Not configured!") );
		return;
	}

	const int slave = ui->slaveID->value();
	const int func = stringToHex(embracedString(ui->functionCode->currentText()));
	const int addr = ui->startAddr->value()-1;
	int num = ui->numCoils->value();
	uint8_t dest[1024];
	uint16_t * dest16 = (uint16_t *) dest;

	memset( dest, 0, 1024 );

	int ret = -1;
	bool is16Bit = false;
	bool writeAccess = false;
	const QString funcType = descriptiveDataTypeName( func );

	modbus_set_slave( m_modbus_snipping, slave );

	switch( func )
	{
		case MODBUS_FC_READ_COILS:
			ret = modbus_read_bits( m_modbus_snipping, addr, num, dest );
			break;
		case MODBUS_FC_READ_DISCRETE_INPUTS:
			ret = modbus_read_input_bits( m_modbus_snipping, addr, num, dest );
			break;
		case MODBUS_FC_READ_HOLDING_REGISTERS:
			ret = modbus_read_registers( m_modbus_snipping, addr, num, dest16 );
			is16Bit = true;
			break;
		case MODBUS_FC_READ_INPUT_REGISTERS:
			ret = modbus_read_input_registers(m_modbus_snipping, addr, num, dest16 );
			is16Bit = true;
			break;
		case MODBUS_FC_WRITE_SINGLE_COIL:
            //ret = modbus_write_bit( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) ? 1 : 0 );
            ret = modbus_write_bit( m_modbus_snipping, addr,ui->radioButton_184->isChecked() ? 1 : 0 );
			writeAccess = true;
			num = 1;
			break;
		case MODBUS_FC_WRITE_SINGLE_REGISTER:
            //ret = modbus_write_register( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) );
            ret = modbus_write_register( m_modbus_snipping, addr,ui->lineEdit_111->text().toInt(0, 0) );
			writeAccess = true;
			num = 1;
			break;
		case MODBUS_FC_WRITE_MULTIPLE_COILS:
		{
			uint8_t * data = new uint8_t[num];
			for( int i = 0; i < num; ++i ) data[i] = ui->regTable->item( i, DataColumn )->text().toInt(0, 0);
			ret = modbus_write_bits( m_modbus_snipping, addr, num, data );
			delete[] data;
			writeAccess = true;
			break;
		}
		case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
		{
            float value;
            QString qvalue = ui->lineEdit_109->text();
            QTextStream floatTextStream(&qvalue);
            floatTextStream >> value;
            quint16 (*reg)[2] = reinterpret_cast<quint16(*)[2]>(&value);
            uint16_t * data = new uint16_t[2];            
            data[0] = (*reg)[1];
            data[1] = (*reg)[0];
            ret = modbus_write_registers( m_modbus_snipping, addr, 2, data );
			delete[] data;
			writeAccess = true;
			break;
		}
		default:
			break;
	}

	if( ret == num  )
	{
		isModbusTransmissionFailed = false;

		if( writeAccess )
		{
			m_statusText->setText(tr( "Values successfully sent" ) );
			m_statusInd->setStyleSheet( "background: #0b0;" );
			m_statusTimer->start( 2000 );
		}
        else
		{
			bool b_hex = is16Bit && ui->checkBoxHexData->checkState() == Qt::Checked;
			QString qs_num;
            QString qs_output = "0x";
            bool ok = false;

			ui->regTable->setRowCount( num );
			for( int i = 0; i < num; ++i )
			{
				int data = is16Bit ? dest16[i] : dest[i];
                QString qs_tmp;

				QTableWidgetItem * dtItem = new QTableWidgetItem( funcType );
				QTableWidgetItem * addrItem = new QTableWidgetItem(QString::number( ui->startAddr->value()+i ) );
				qs_num.sprintf( b_hex ? "0x%04x" : "%d", data);
				qs_tmp.sprintf("%04x", data);
                qs_output.append(qs_tmp);
				QTableWidgetItem * dataItem = new QTableWidgetItem( qs_num );
				dtItem->setFlags( dtItem->flags() & ~Qt::ItemIsEditable );
				addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
				dataItem->setFlags( dataItem->flags() & ~Qt::ItemIsEditable );
				ui->regTable->setItem( i, DataTypeColumn, dtItem );
				ui->regTable->setItem( i, AddrColumn, addrItem );
				ui->regTable->setItem( i, DataColumn, dataItem );
                if (ui->radioButton_182->isChecked()) ui->lineEdit_111->setText(QString::number(data));
                else if (ui->radioButton_183->isChecked())
                {
                    (data) ? ui->radioButton_184->setChecked(true) : ui->radioButton_185->setChecked(true);
                }
            }

            QByteArray array = QByteArray::fromHex(qs_output.toLatin1());
            const float d = toFloat(array);

            if (ui->radioButton_181->isChecked())
            {
                (b_hex) ? ui->lineEdit_109->setText(qs_output) : ui->lineEdit_109->setText(QString::number(d,'f',10)) ;
            }
		}
	}
	else
	{
		QString err;

		if( ret < 0 )
		{
			if(
#ifdef WIN32
					errno == WSAETIMEDOUT ||
#endif
					errno == EIO
																	)
			{
				err += tr( "I/O error" );
				err += ": ";
				err += tr( "did not receive any data from slave." );
			}
			else
			{
				err += tr( "Protocol error" );
				err += ": ";
				err += tr( "Slave threw exception '" );
				err += modbus_strerror( errno );
				err += tr( "' or function not implemented." );
			}
		}
		else
		{
			err += tr( "Protocol error" );
			err += ": ";
			err += tr( "Number of registers returned does not "
					"match number of registers requested!" );
		}

		if( err.size() > 0 )
			setStatusError( err );

		isModbusTransmissionFailed = true;
	}
}


void MainWindow::resetStatus( void )
{
	m_statusText->setText( tr( "Ready" ) );
	m_statusInd->setStyleSheet( "background: #aaa;" );
}

void MainWindow::pollForDataOnBus( void )
{
	if( m_modbus )
	{
		modbus_poll( m_modbus );
	}
}


void MainWindow::openBatchProcessor()
{
	BatchProcessor( this, m_modbus ).exec();
}


void MainWindow::aboutQModBus( void )
{
	AboutDialog( this ).exec();
}

void MainWindow::onRtuPortActive(bool active)
{
	if (active) {
        m_modbus = this->modbus();
		if (m_modbus) {
			modbus_register_monitor_add_item_fnc(m_modbus, MainWindow::stBusMonitorAddItem);
			modbus_register_monitor_raw_data_fnc(m_modbus, MainWindow::stBusMonitorRawData);
		}
	}
	else {
		m_modbus = NULL;
	}
}

void MainWindow::onRtuPortActive_2(bool active)
{
    if (active) {
        m_modbus_2 = this->modbus_2();
        if (m_modbus_2) {
            modbus_register_monitor_add_item_fnc(m_modbus_2, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(m_modbus_2, MainWindow::stBusMonitorRawData);
        }
    }
    else {
        m_modbus_2 = NULL;
    }
}

void MainWindow::onRtuPortActive_3(bool active)
{
    if (active) {
        m_modbus_3 = this->modbus_3();
        if (m_modbus_3) {
            modbus_register_monitor_add_item_fnc(m_modbus_3, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(m_modbus_3, MainWindow::stBusMonitorRawData);
        }
    }
    else {
        m_modbus_3 = NULL;
    }
}

void MainWindow::onRtuPortActive_4(bool active)
{
    if (active) {
        m_modbus_4 = this->modbus_4();
        if (m_modbus_4) {
            modbus_register_monitor_add_item_fnc(m_modbus_4, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(m_modbus_4, MainWindow::stBusMonitorRawData);
        }
    }
    else {
        m_modbus_4 = NULL;
    }
}

void MainWindow::onRtuPortActive_5(bool active)
{
    if (active) {
        m_modbus_5 = this->modbus_5();
        if (m_modbus_5) {
            modbus_register_monitor_add_item_fnc(m_modbus_5, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(m_modbus_5, MainWindow::stBusMonitorRawData);
        }
    }
    else {
        m_modbus_5 = NULL;
    }
}

void MainWindow::onRtuPortActive_6(bool active)
{
    if (active) {
        m_modbus_6 = this->modbus_6();
        if (m_modbus_6) {
            modbus_register_monitor_add_item_fnc(m_modbus_6, MainWindow::stBusMonitorAddItem);
            modbus_register_monitor_raw_data_fnc(m_modbus_6, MainWindow::stBusMonitorRawData);
        }
    }
    else {
        m_modbus_6 = NULL;
    }
}

void
MainWindow::
setStatusError(const QString &msg)
{
    m_statusText->setText( msg );
    m_statusInd->setStyleSheet( "background: red;" );
    m_statusTimer->start( 2000 );
}


void
MainWindow::
updateGraph()
{
    chart = new QChart();
    chart->legend()->hide();

    axisX = new QValueAxis;
    axisX->setRange(0,1000);
    axisX->setTickCount(11);
    axisX->setTickInterval(100);
    axisX->setLabelFormat("%i");
    axisX->setTitleText("Frequency (Mhz)");

    axisY = new QValueAxis;
    axisY->setRange(0,100);
    axisY->setTickCount(11);
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Watercut (%)");

    axisY3 = new QValueAxis;
    axisY3->setRange(0,2.5);
    axisY3->setTickCount(11);
    axisY3->setLabelFormat("%.1f");
    axisY3->setTitleText("Reflected Power (V)");

    chart->addAxis(axisX, Qt::AlignBottom);

    series = new QSplineSeries;
    axisY->setLinePenColor(series->pen().color());
    axisY->setLabelsColor(series->pen().color());
    *series << QPointF(100, 5) << QPointF(300, 48) << QPointF(600, 68) << QPointF(800, 89);
    chart->addSeries(series);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    series = new QSplineSeries;
    axisY3->setLinePenColor(series->pen().color());
    axisY3->setLabelsColor(series->pen().color());
    *series << QPointF(1, 0.5) << QPointF(105, 1.5) << QPointF(400.4, 1.6) << QPointF(500.3, 1.7) << QPointF(600.2, 1.8) << QPointF(700.4, 2.0) << QPointF(800.3, 2.1) << QPointF(900, 2.4);
    chart->addSeries(series);
    chart->addAxis(axisY3, Qt::AlignRight);
    series->attachAxis(axisX);
    series->attachAxis(axisY3);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    updateChartTitle();

    ui->gridLayout_5->addWidget(chartView,0,0);
}

void
MainWindow::
initializeFrequencyGauge()
{
    m_frequencyGauge = new QcGaugeWidget;
    m_frequencyGauge->addBackground(99);
    QcBackgroundItem *bkg1 = m_frequencyGauge->addBackground(92);
    bkg1->clearrColors();
    bkg1->addColor(0.1,Qt::black);
    bkg1->addColor(1.0,Qt::white);

    QcBackgroundItem *bkg2 = m_frequencyGauge->addBackground(88);
    bkg2->clearrColors();
    bkg2->addColor(0.1,Qt::gray);
    bkg2->addColor(1.0,Qt::darkGray);

    m_frequencyGauge->addArc(55);
    m_frequencyGauge->addDegrees(65)->setValueRange(0,80);
    m_frequencyGauge->addColorBand(50);
    m_frequencyGauge->addValues(80)->setValueRange(0,80);
    m_frequencyGauge->addLabel(70)->setText("Freq (Mhz)");
    QcLabelItem *lab = m_frequencyGauge->addLabel(40);
    lab->setText("0");
    m_frequencyNeedle = m_frequencyGauge->addNeedle(60);
    m_frequencyNeedle->setLabel(lab);
    m_frequencyNeedle->setColor(Qt::white);
    m_frequencyNeedle->setValueRange(0,80);
    m_frequencyGauge->addBackground(7);
    m_frequencyGauge->addGlass(88);
    ui->gridLayout_6->addWidget(m_frequencyGauge);
}


void
MainWindow::
updateFrequencyGauge()
{}


void
MainWindow::
initializeTemperatureGauge()
{
    m_temperatureGauge = new QcGaugeWidget;
    m_temperatureGauge->addBackground(99);
    QcBackgroundItem *bkg1 = m_temperatureGauge->addBackground(92);
    bkg1->clearrColors();
    bkg1->addColor(0.1,Qt::black);
    bkg1->addColor(1.0,Qt::white);

    QcBackgroundItem *bkg2 = m_temperatureGauge->addBackground(88);
    bkg2->clearrColors();
    bkg2->addColor(0.1,Qt::gray);
    bkg2->addColor(1.0,Qt::darkGray);

    m_temperatureGauge->addArc(55);
    m_temperatureGauge->addDegrees(65)->setValueRange(0,80);
    m_temperatureGauge->addColorBand(50);
    m_temperatureGauge->addValues(80)->setValueRange(0,80);
    m_temperatureGauge->addLabel(70)->setText("Temp (C°)");
    QcLabelItem *lab = m_temperatureGauge->addLabel(40);
    lab->setText("0");
    m_temperatureNeedle = m_temperatureGauge->addNeedle(60);
    m_temperatureNeedle->setLabel(lab);
    m_temperatureNeedle->setColor(Qt::white);
    m_temperatureNeedle->setValueRange(0,80);
    m_temperatureGauge->addBackground(7);
    m_temperatureGauge->addGlass(88);
    ui->gridLayout_7->addWidget(m_temperatureGauge);
}

void
MainWindow::
updateTemperatureGauge()
{}

void
MainWindow::
initializeDensityGauge()
{
    m_densityGauge = new QcGaugeWidget;
    m_densityGauge->addBackground(99);
    QcBackgroundItem *bkg1 = m_densityGauge->addBackground(92);
    bkg1->clearrColors();
    bkg1->addColor(0.1,Qt::black);
    bkg1->addColor(1.0,Qt::white);

    QcBackgroundItem *bkg2 = m_densityGauge->addBackground(88);
    bkg2->clearrColors();
    bkg2->addColor(0.1,Qt::gray);
    bkg2->addColor(1.0,Qt::darkGray);

    m_densityGauge->addArc(55);
    m_densityGauge->addDegrees(65)->setValueRange(0,80);
    m_densityGauge->addColorBand(50);
    m_densityGauge->addValues(80)->setValueRange(0,80);
    m_densityGauge->addLabel(70)->setText("Density");
    QcLabelItem *lab = m_densityGauge->addLabel(40);
    lab->setText("0");
    m_densityNeedle = m_densityGauge->addNeedle(60);
    m_densityNeedle->setLabel(lab);
    m_densityNeedle->setColor(Qt::white);
    m_densityNeedle->setValueRange(0,80);
    m_densityGauge->addBackground(7);
    m_densityGauge->addGlass(88);
    ui->gridLayout_8->addWidget(m_densityGauge);
}

void
MainWindow::
updateDensityGauge()
{}

void
MainWindow::
initializeRPGauge()
{
    m_RPGauge = new QcGaugeWidget;
    m_RPGauge->addBackground(99);
    QcBackgroundItem *bkg1 = m_RPGauge->addBackground(92);
    bkg1->clearrColors();
    bkg1->addColor(0.1,Qt::black);
    bkg1->addColor(1.0,Qt::white);

    QcBackgroundItem *bkg2 = m_RPGauge->addBackground(88);
    bkg2->clearrColors();
    bkg2->addColor(0.1,Qt::gray);
    bkg2->addColor(1.0,Qt::darkGray);

    m_RPGauge->addArc(55);
    m_RPGauge->addDegrees(65)->setValueRange(0,80);
    m_RPGauge->addColorBand(50);
    m_RPGauge->addValues(80)->setValueRange(0,80);
    m_RPGauge->addLabel(70)->setText("RP (V)");
    QcLabelItem *lab = m_RPGauge->addLabel(40);
    lab->setText("0");
    m_RPNeedle = m_RPGauge->addNeedle(60);
    m_RPNeedle->setLabel(lab);
    m_RPNeedle->setColor(Qt::white);
    m_RPNeedle->setValueRange(0,80);
    m_RPGauge->addBackground(7);
    m_RPGauge->addGlass(88);
    ui->gridLayout_9->addWidget(m_RPGauge);
}

void
MainWindow::
updateRPGauge()
{}

/// L1P1
void
MainWindow::
onRadioButtonPressed()
{
    /// analyzer
    ui->groupBox_6->setEnabled(TRUE);
    ui->groupBox_125->setEnabled(TRUE);

    updateRegisters(EEA,0);
}

void
MainWindow::
onRadioButton_2Pressed()
{
    /// razor
    ui->groupBox_6->setDisabled(TRUE);
    ui->groupBox_125->setDisabled(TRUE);

    ui->radioButton_5->setChecked(TRUE); // midcut
    ui->radioButton_7->setChecked(TRUE); // 1 oscillator

    updateRegisters(RAZ,0);
}

void
MainWindow::
onRadioButton_3Pressed()
{

}

void
MainWindow::
onRadioButton_4Pressed()
{

}

void
MainWindow::
onRadioButton_5Pressed()
{

}

void
MainWindow::
onRadioButton_6Pressed()
{

}

void
MainWindow::
onRadioButton_7Pressed()
{

}

void
MainWindow::
onRadioButton_8Pressed()
{

}
void
MainWindow::
onRadioButton_9Pressed()
{

}

void
MainWindow::
onRadioButton_10Pressed()
{

}

void
MainWindow::
onRadioButton_11Pressed()
{

}

void
MainWindow::
onRadioButton_12Pressed()
{

}


/// L1P2
void
MainWindow::
onRadioButton_13Pressed()
{
    ui->groupBox_13->setEnabled(TRUE);
    ui->groupBox_126->setEnabled(TRUE);

    updateRegisters(EEA,1);
}

void
MainWindow::
onRadioButton_22Pressed()
{
    ui->radioButton_37->setChecked(TRUE);
    ui->radioButton_39->setChecked(TRUE);

    ui->groupBox_13->setDisabled(TRUE);
    ui->groupBox_126->setDisabled(TRUE);

    updateRegisters(RAZ,1);
}

void
MainWindow::
onRadioButton_23Pressed()
{

}

void
MainWindow::
onRadioButton_24Pressed()
{

}

void
MainWindow::
onRadioButton_37Pressed()
{

}

void
MainWindow::
onRadioButton_38Pressed()
{

}

void
MainWindow::
onRadioButton_39Pressed()
{

}

void
MainWindow::
onRadioButton_40Pressed()
{

}

void
MainWindow::
onRadioButton_41Pressed()
{

}
void
MainWindow::
onRadioButton_42Pressed()
{

}

void
MainWindow::
onRadioButton_43Pressed()
{

}

void
MainWindow::
onRadioButton_44Pressed()
{

}



/// L1P3
void
MainWindow::
onRadioButton_69Pressed()
{
    ui->groupBox_34->setEnabled(TRUE);
    ui->groupBox_131->setEnabled(TRUE);

    updateRegisters(EEA,2);
}

void
MainWindow::
onRadioButton_70Pressed()
{
    ui->radioButton_73->setChecked(TRUE);
    ui->radioButton_75->setChecked(TRUE);

    ui->groupBox_34->setDisabled(TRUE);
    ui->groupBox_131->setDisabled(TRUE);

    updateRegisters(RAZ,2);
}

void
MainWindow::
onRadioButton_71Pressed()
{
}

void
MainWindow::
onRadioButton_72Pressed()
{
}

void
MainWindow::
onRadioButton_73Pressed()
{
}

void
MainWindow::
onRadioButton_74Pressed()
{
}


void
MainWindow::
onRadioButton_75Pressed()
{
}

void
MainWindow::
onRadioButton_76Pressed()
{
}


void
MainWindow::
onRadioButton_77Pressed()
{
}

void
MainWindow::
onRadioButton_78Pressed()
{
}


void
MainWindow::
onRadioButton_79Pressed()
{
}

void
MainWindow::
onRadioButton_80Pressed()
{
}


/// L2P1
void
MainWindow::
onRadioButton_81Pressed()
{
    ui->groupBox_41->setEnabled(TRUE);
    ui->groupBox_133->setEnabled(TRUE);
    updateRegisters(EEA,3);
}

void
MainWindow::
onRadioButton_82Pressed()
{
    ui->radioButton_85->setChecked(TRUE);
    ui->radioButton_87->setChecked(TRUE);
    ui->groupBox_41->setDisabled(TRUE);
    ui->groupBox_133->setDisabled(TRUE);
    updateRegisters(RAZ,3);
}

void
MainWindow::
onRadioButton_83Pressed()
{
}

void
MainWindow::
onRadioButton_84Pressed()
{
}

void
MainWindow::
onRadioButton_85Pressed()
{
}

void
MainWindow::
onRadioButton_86Pressed()
{
}

void
MainWindow::
onRadioButton_87Pressed()
{
}

void
MainWindow::
onRadioButton_88Pressed()
{
}

void
MainWindow::
onRadioButton_89Pressed()
{
}

void
MainWindow::
onRadioButton_90Pressed()
{
}

void
MainWindow::
onRadioButton_91Pressed()
{
}

void
MainWindow::
onRadioButton_92Pressed()
{
}


/// L2P2
void
MainWindow::
onRadioButton_93Pressed()
{
    ui->groupBox_47->setEnabled(TRUE);
    ui->groupBox_137->setEnabled(TRUE);

    updateRegisters(EEA,4);
}

void
MainWindow::
onRadioButton_94Pressed()
{
    ui->radioButton_97->setChecked(TRUE);
    ui->radioButton_99->setChecked(TRUE);
    ui->groupBox_47->setDisabled(TRUE);
    ui->groupBox_137->setDisabled(TRUE);

    updateRegisters(RAZ,4);
}

void
MainWindow::
onRadioButton_95Pressed()
{
}

void
MainWindow::
onRadioButton_96Pressed()
{
}

void
MainWindow::
onRadioButton_97Pressed()
{
}

void
MainWindow::
onRadioButton_98Pressed()
{
}

void
MainWindow::
onRadioButton_99Pressed()
{
}

void
MainWindow::
onRadioButton_100Pressed()
{
}

void
MainWindow::
onRadioButton_101Pressed()
{
}

void
MainWindow::
onRadioButton_102Pressed()
{
}

void
MainWindow::
onRadioButton_103Pressed()
{
}

void
MainWindow::
onRadioButton_104Pressed()
{
}


/// L2P3
void
MainWindow::
onRadioButton_105Pressed()
{
    ui->groupBox_54->setEnabled(TRUE);
    ui->groupBox_140->setEnabled(TRUE);

    updateRegisters(EEA,5);
}

void
MainWindow::
onRadioButton_106Pressed()
{
    ui->radioButton_109->setChecked(TRUE);
    ui->radioButton_111->setChecked(TRUE);
    ui->groupBox_54->setDisabled(TRUE);
    ui->groupBox_140->setDisabled(TRUE);
    
    updateRegisters(RAZ,5);
}

void
MainWindow::
onRadioButton_107Pressed()
{
}

void
MainWindow::
onRadioButton_108Pressed()
{
}

void
MainWindow::
onRadioButton_109Pressed()
{
}

void
MainWindow::
onRadioButton_110Pressed()
{
}

void
MainWindow::
onRadioButton_111Pressed()
{
}

void
MainWindow::
onRadioButton_112Pressed()
{
}

void
MainWindow::
onRadioButton_113Pressed()
{
}

void
MainWindow::
onRadioButton_114Pressed()
{
}

void
MainWindow::
onRadioButton_115Pressed()
{
}

void
MainWindow::
onRadioButton_116Pressed()
{
}


/// L3P1
void
MainWindow::
onRadioButton_117Pressed()
{
    ui->groupBox_61->setEnabled(TRUE);
    ui->groupBox_142->setEnabled(TRUE);

    updateRegisters(EEA,6);
}

void
MainWindow::
onRadioButton_118Pressed()
{
    ui->radioButton_121->setChecked(TRUE);
    ui->radioButton_123->setChecked(TRUE);
    ui->groupBox_61->setDisabled(TRUE);
    ui->groupBox_142->setDisabled(TRUE);

    updateRegisters(RAZ,6);
}

void
MainWindow::
onRadioButton_119Pressed()
{
}

void
MainWindow::
onRadioButton_120Pressed()
{
}

void
MainWindow::
onRadioButton_121Pressed()
{
}

void
MainWindow::
onRadioButton_122Pressed()
{
}

void
MainWindow::
onRadioButton_123Pressed()
{
}

void
MainWindow::
onRadioButton_124Pressed()
{
}

void
MainWindow::
onRadioButton_125Pressed()
{
}

void
MainWindow::
onRadioButton_126Pressed()
{
}

void
MainWindow::
onRadioButton_127Pressed()
{
}

void
MainWindow::
onRadioButton_128Pressed()
{
}


/// L3P2
void
MainWindow::
onRadioButton_129Pressed()
{
    ui->groupBox_77->setEnabled(TRUE);
    ui->groupBox_146->setEnabled(TRUE);

    updateRegisters(EEA,7);
}

void
MainWindow::
onRadioButton_130Pressed()
{
    ui->radioButton_133->setChecked(TRUE);
    ui->radioButton_135->setChecked(TRUE);
    ui->groupBox_77->setDisabled(TRUE);
    ui->groupBox_146->setDisabled(TRUE);

    updateRegisters(RAZ,7);
}

void
MainWindow::
onRadioButton_131Pressed()
{
}

void
MainWindow::
onRadioButton_132Pressed()
{
}

void
MainWindow::
onRadioButton_133Pressed()
{
}

void
MainWindow::
onRadioButton_134Pressed()
{
}

void
MainWindow::
onRadioButton_135Pressed()
{
}

void
MainWindow::
onRadioButton_136Pressed()
{
}

void
MainWindow::
onRadioButton_137Pressed()
{
}

void
MainWindow::
onRadioButton_138Pressed()
{
}

void
MainWindow::
onRadioButton_139Pressed()
{
}

void
MainWindow::
onRadioButton_140Pressed()
{
}


/// L3P3
void
MainWindow::
onRadioButton_141Pressed()
{
    ui->groupBox_84->setEnabled(TRUE);
    ui->groupBox_149->setEnabled(TRUE);

    updateRegisters(EEA,8);
}

void
MainWindow::
onRadioButton_142Pressed()
{
    ui->radioButton_145->setChecked(TRUE);
    ui->radioButton_147->setChecked(TRUE);
    ui->groupBox_84->setDisabled(TRUE);
    ui->groupBox_149->setDisabled(TRUE);

    updateRegisters(RAZ,8);
}

void
MainWindow::
onRadioButton_143Pressed()
{
}

void
MainWindow::
onRadioButton_144Pressed()
{
}

void
MainWindow::
onRadioButton_145Pressed()
{
}

void
MainWindow::
onRadioButton_146Pressed()
{
}

void
MainWindow::
onRadioButton_147Pressed()
{
}

void
MainWindow::
onRadioButton_148Pressed()
{
}

void
MainWindow::
onRadioButton_149Pressed()
{
}

void
MainWindow::
onRadioButton_150Pressed()
{
}

void
MainWindow::
onRadioButton_151Pressed()
{
}

void
MainWindow::
onRadioButton_152Pressed()
{
}


/// L4P1
void
MainWindow::
onRadioButton_153Pressed()
{
    ui->groupBox_91->setEnabled(TRUE);
    ui->groupBox_151->setEnabled(TRUE);

    updateRegisters(EEA,9);
}

void
MainWindow::
onRadioButton_154Pressed()
{
    ui->radioButton_157->setChecked(TRUE);
    ui->radioButton_159->setChecked(TRUE);
    ui->groupBox_91->setDisabled(TRUE);
    ui->groupBox_151->setDisabled(TRUE);

    updateRegisters(RAZ,9);
}

void
MainWindow::
onRadioButton_155Pressed()
{
}

void
MainWindow::
onRadioButton_156Pressed()
{
}

void
MainWindow::
onRadioButton_157Pressed()
{
}

void
MainWindow::
onRadioButton_158Pressed()
{
}

void
MainWindow::
onRadioButton_159Pressed()
{
}

void
MainWindow::
onRadioButton_160Pressed()
{
}

void
MainWindow::
onRadioButton_161Pressed()
{
}

void
MainWindow::
onRadioButton_162Pressed()
{
}

void
MainWindow::
onRadioButton_163Pressed()
{
}

void
MainWindow::
onRadioButton_164Pressed()
{
}


/// L4P2
void
MainWindow::
onRadioButton_165Pressed()
{
    ui->groupBox_98->setEnabled(TRUE);
    ui->groupBox_155->setEnabled(TRUE);

    updateRegisters(EEA,10);
}

void
MainWindow::
onRadioButton_166Pressed()
{
    ui->radioButton_169->setChecked(TRUE);
    ui->radioButton_171->setChecked(TRUE);
    ui->groupBox_98->setDisabled(TRUE);
    ui->groupBox_155->setDisabled(TRUE);

    updateRegisters(RAZ,10);
}

void
MainWindow::
onRadioButton_167Pressed()
{
}

void
MainWindow::
onRadioButton_168Pressed()
{
}

void
MainWindow::
onRadioButton_169Pressed()
{
}

void
MainWindow::
onRadioButton_170Pressed()
{
}

void
MainWindow::
onRadioButton_171Pressed()
{
}

void
MainWindow::
onRadioButton_172Pressed()
{
}

void
MainWindow::
onRadioButton_173Pressed()
{
}

void
MainWindow::
onRadioButton_174Pressed()
{
}

void
MainWindow::
onRadioButton_175Pressed()
{
}

void
MainWindow::
onRadioButton_176Pressed()
{
}


/// L4P3
void
MainWindow::
onRadioButton_177Pressed()
{
    ui->groupBox_158->setEnabled(TRUE);
    ui->groupBox_159->setEnabled(TRUE);

    updateRegisters(EEA,11);
}

void
MainWindow::
onRadioButton_178Pressed()
{
    ui->radioButton_194->setChecked(TRUE);
    ui->radioButton_196->setChecked(TRUE);
    ui->groupBox_158->setDisabled(TRUE);
    ui->groupBox_159->setDisabled(TRUE);

    updateRegisters(RAZ,11);
}

void
MainWindow::
onRadioButton_179Pressed()
{
}

void
MainWindow::
onRadioButton_180Pressed()
{
}

void
MainWindow::
onRadioButton_194Pressed()
{
}

void
MainWindow::
onRadioButton_195Pressed()
{
}

void
MainWindow::
onRadioButton_196Pressed()
{
}

void
MainWindow::
onRadioButton_197Pressed()
{
}

void
MainWindow::
onRadioButton_198Pressed()
{
}

void
MainWindow::
onRadioButton_199Pressed()
{
}

void
MainWindow::
onRadioButton_200Pressed()
{
}

void
MainWindow::
onRadioButton_201Pressed()
{
}


/// L5P1
void
MainWindow::
onRadioButton_202Pressed()
{
    ui->groupBox_167->setEnabled(TRUE);
    ui->groupBox_168->setEnabled(TRUE);

    updateRegisters(EEA,12);
}

void
MainWindow::
onRadioButton_203Pressed()
{
    ui->radioButton_206->setChecked(TRUE);
    ui->radioButton_208->setChecked(TRUE);
    ui->groupBox_167->setDisabled(TRUE);
    ui->groupBox_168->setDisabled(TRUE);

    updateRegisters(RAZ,12);
}

void
MainWindow::
onRadioButton_204Pressed()
{
}

void
MainWindow::
onRadioButton_205Pressed()
{
}

void
MainWindow::
onRadioButton_206Pressed()
{
}

void
MainWindow::
onRadioButton_207Pressed()
{
}

void
MainWindow::
onRadioButton_208Pressed()
{
}

void
MainWindow::
onRadioButton_209Pressed()
{
}

void
MainWindow::
onRadioButton_210Pressed()
{
}

void
MainWindow::
onRadioButton_211Pressed()
{
}

void
MainWindow::
onRadioButton_212Pressed()
{
}

void
MainWindow::
onRadioButton_213Pressed()
{
}


/// L5P2
void
MainWindow::
onRadioButton_214Pressed()
{
    ui->groupBox_178->setEnabled(TRUE);
    ui->groupBox_179->setEnabled(TRUE);

    updateRegisters(EEA,13);
}

void
MainWindow::
onRadioButton_215Pressed()
{
    ui->radioButton_218->setChecked(TRUE);
    ui->radioButton_220->setChecked(TRUE);
    ui->groupBox_178->setDisabled(TRUE);
    ui->groupBox_179->setDisabled(TRUE);

    updateRegisters(RAZ,13);
}

void
MainWindow::
onRadioButton_216Pressed()
{
}

void
MainWindow::
onRadioButton_217Pressed()
{
}

void
MainWindow::
onRadioButton_218Pressed()
{
}

void
MainWindow::
onRadioButton_219Pressed()
{
}

void
MainWindow::
onRadioButton_220Pressed()
{
}

void
MainWindow::
onRadioButton_221Pressed()
{
}

void
MainWindow::
onRadioButton_222Pressed()
{
}

void
MainWindow::
onRadioButton_223Pressed()
{
}

void
MainWindow::
onRadioButton_224Pressed()
{
}

void
MainWindow::
onRadioButton_225Pressed()
{
}


/// L5P3
void
MainWindow::
onRadioButton_226Pressed()
{
    ui->groupBox_188->setEnabled(TRUE);
    ui->groupBox_189->setEnabled(TRUE);

    updateRegisters(EEA,14);
}

void
MainWindow::
onRadioButton_227Pressed()
{
    ui->radioButton_230->setChecked(TRUE);
    ui->radioButton_232->setChecked(TRUE);
    ui->groupBox_188->setDisabled(TRUE);
    ui->groupBox_189->setDisabled(TRUE);

    updateRegisters(RAZ,14);
}

void
MainWindow::
onRadioButton_228Pressed()
{
}

void
MainWindow::
onRadioButton_229Pressed()
{
}

void
MainWindow::
onRadioButton_230Pressed()
{
}

void
MainWindow::
onRadioButton_231Pressed()
{
}

void
MainWindow::
onRadioButton_232Pressed()
{
}

void
MainWindow::
onRadioButton_233Pressed()
{
}

void
MainWindow::
onRadioButton_234Pressed()
{
}

void
MainWindow::
onRadioButton_235Pressed()
{
}

void
MainWindow::
onRadioButton_236Pressed()
{
}

void
MainWindow::
onRadioButton_237Pressed()
{
}


/// L6P1
void
MainWindow::
onRadioButton_238Pressed()
{
    ui->groupBox_197->setEnabled(TRUE);
    ui->groupBox_198->setEnabled(TRUE);

    updateRegisters(EEA,15);
}

void
MainWindow::
onRadioButton_239Pressed()
{
    ui->radioButton_242->setChecked(TRUE);
    ui->radioButton_244->setChecked(TRUE);
    ui->groupBox_197->setDisabled(TRUE);
    ui->groupBox_198->setDisabled(TRUE);

    updateRegisters(RAZ,15);
}

void
MainWindow::
onRadioButton_240Pressed()
{
}

void
MainWindow::
onRadioButton_241Pressed()
{
}

void
MainWindow::
onRadioButton_242Pressed()
{
}

void
MainWindow::
onRadioButton_243Pressed()
{
}

void
MainWindow::
onRadioButton_244Pressed()
{
}

void
MainWindow::
onRadioButton_245Pressed()
{
}

void
MainWindow::
onRadioButton_246Pressed()
{
}

void
MainWindow::
onRadioButton_247Pressed()
{
}

void
MainWindow::
onRadioButton_248Pressed()
{
}

void
MainWindow::
onRadioButton_249Pressed()
{
}


/// L6P2
void
MainWindow::
onRadioButton_250Pressed()
{
    ui->groupBox_208->setEnabled(TRUE);
    ui->groupBox_209->setEnabled(TRUE);

    updateRegisters(EEA,16);
}

void
MainWindow::
onRadioButton_251Pressed()
{
    ui->radioButton_254->setChecked(TRUE);
    ui->radioButton_256->setChecked(TRUE);
    ui->groupBox_208->setDisabled(TRUE);
    ui->groupBox_209->setDisabled(TRUE);

    updateRegisters(RAZ,16);
}

void
MainWindow::
onRadioButton_252Pressed()
{
}

void
MainWindow::
onRadioButton_253Pressed()
{
}

void
MainWindow::
onRadioButton_254Pressed()
{
}

void
MainWindow::
onRadioButton_255Pressed()
{
}

void
MainWindow::
onRadioButton_256Pressed()
{
}

void
MainWindow::
onRadioButton_257Pressed()
{
}

void
MainWindow::
onRadioButton_258Pressed()
{
}

void
MainWindow::
onRadioButton_259Pressed()
{
}

void
MainWindow::
onRadioButton_260Pressed()
{
}

void
MainWindow::
onRadioButton_261Pressed()
{
}


/// L6P3
void
MainWindow::
onRadioButton_262Pressed()
{
    ui->groupBox_218->setEnabled(TRUE);
    ui->groupBox_219->setEnabled(TRUE);

    updateRegisters(EEA,17);
}

void
MainWindow::
onRadioButton_263Pressed()
{
    ui->radioButton_266->setChecked(TRUE);
    ui->radioButton_268->setChecked(TRUE);
    ui->groupBox_218->setDisabled(TRUE);
    ui->groupBox_219->setDisabled(TRUE);

    updateRegisters(RAZ,17);
}

void
MainWindow::
onRadioButton_264Pressed()
{
}

void
MainWindow::
onRadioButton_265Pressed()
{
}

void
MainWindow::
onRadioButton_266Pressed()
{
}

void
MainWindow::
onRadioButton_267Pressed()
{
}

void
MainWindow::
onRadioButton_268Pressed()
{
}

void
MainWindow::
onRadioButton_269Pressed()
{
}

void
MainWindow::
onRadioButton_270Pressed()
{
}

void
MainWindow::
onRadioButton_271Pressed()
{
}

void
MainWindow::
onRadioButton_272Pressed()
{
}

void
MainWindow::
onRadioButton_273Pressed()
{
}


void
MainWindow::
connectTimers()
{
    QTimer * t = new QTimer( this );
    connect( t, SIGNAL(timeout()), this, SLOT(pollForDataOnBus()));
    t->start( 5 );

    m_pollTimer = new QTimer( this );
    connect( m_pollTimer, SIGNAL(timeout()), this, SLOT(sendModbusRequest()));

    m_statusTimer = new QTimer( this );
    connect( m_statusTimer, SIGNAL(timeout()), this, SLOT(resetStatus()));
    m_statusTimer->setSingleShot(true);
}


void
MainWindow::
connectLoopDependentData()
{
    connect(ui->tabWidget_2, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_3, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_4, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_5, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_6, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_7, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
    connect(ui->tabWidget_8, SIGNAL(currentChanged(int)), this, SLOT(onLoopTabChanged(int)));
}


void
MainWindow::
connectRadioButtons()
{
    // L1P1
    connect(ui->radioButton, SIGNAL(pressed()), this, SLOT(onRadioButtonPressed()));
    connect(ui->radioButton_2, SIGNAL(pressed()), this, SLOT(onRadioButton_2Pressed()));
    connect(ui->radioButton_3, SIGNAL(pressed()), this, SLOT(onRadioButton_3Pressed()));
    connect(ui->radioButton_4, SIGNAL(pressed()), this, SLOT(onRadioButton_4Pressed()));
    connect(ui->radioButton_5, SIGNAL(pressed()), this, SLOT(onRadioButton_5Pressed()));
    connect(ui->radioButton_6, SIGNAL(pressed()), this, SLOT(onRadioButton_6Pressed()));
    connect(ui->radioButton_7, SIGNAL(pressed()), this, SLOT(onRadioButton_7Pressed()));
    connect(ui->radioButton_8, SIGNAL(pressed()), this, SLOT(onRadioButton_8Pressed()));
    connect(ui->radioButton_9, SIGNAL(pressed()), this, SLOT(onRadioButton_9Pressed()));
    connect(ui->radioButton_10, SIGNAL(pressed()), this, SLOT(onRadioButton_10Pressed()));
    connect(ui->radioButton_11, SIGNAL(pressed()), this, SLOT(onRadioButton_11Pressed()));
    connect(ui->radioButton_12, SIGNAL(pressed()), this, SLOT(onRadioButton_12Pressed()));

    // L1P2
    connect(ui->radioButton_13, SIGNAL(pressed()), this, SLOT(onRadioButton_13Pressed()));
    connect(ui->radioButton_22, SIGNAL(pressed()), this, SLOT(onRadioButton_22Pressed()));
    connect(ui->radioButton_23, SIGNAL(pressed()), this, SLOT(onRadioButton_23Pressed()));
    connect(ui->radioButton_24, SIGNAL(pressed()), this, SLOT(onRadioButton_24Pressed()));
    connect(ui->radioButton_37, SIGNAL(pressed()), this, SLOT(onRadioButton_37Pressed()));
    connect(ui->radioButton_38, SIGNAL(pressed()), this, SLOT(onRadioButton_38Pressed()));
    connect(ui->radioButton_39, SIGNAL(pressed()), this, SLOT(onRadioButton_39Pressed()));
    connect(ui->radioButton_40, SIGNAL(pressed()), this, SLOT(onRadioButton_40Pressed()));
    connect(ui->radioButton_41, SIGNAL(pressed()), this, SLOT(onRadioButton_41Pressed()));
    connect(ui->radioButton_42, SIGNAL(pressed()), this, SLOT(onRadioButton_42Pressed()));
    connect(ui->radioButton_43, SIGNAL(pressed()), this, SLOT(onRadioButton_43Pressed()));
    connect(ui->radioButton_44, SIGNAL(pressed()), this, SLOT(onRadioButton_44Pressed()));


    // L1P3
    connect(ui->radioButton_69, SIGNAL(pressed()), this, SLOT(onRadioButton_69Pressed()));
    connect(ui->radioButton_70, SIGNAL(pressed()), this, SLOT(onRadioButton_70Pressed()));
    connect(ui->radioButton_72, SIGNAL(pressed()), this, SLOT(onRadioButton_72Pressed()));
    connect(ui->radioButton_73, SIGNAL(pressed()), this, SLOT(onRadioButton_73Pressed()));
    connect(ui->radioButton_74, SIGNAL(pressed()), this, SLOT(onRadioButton_74Pressed()));
    connect(ui->radioButton_71, SIGNAL(pressed()), this, SLOT(onRadioButton_71Pressed()));
    connect(ui->radioButton_75, SIGNAL(pressed()), this, SLOT(onRadioButton_75Pressed()));
    connect(ui->radioButton_76, SIGNAL(pressed()), this, SLOT(onRadioButton_78Pressed()));
    connect(ui->radioButton_77, SIGNAL(pressed()), this, SLOT(onRadioButton_77Pressed()));
    connect(ui->radioButton_78, SIGNAL(pressed()), this, SLOT(onRadioButton_78Pressed()));
    connect(ui->radioButton_79, SIGNAL(pressed()), this, SLOT(onRadioButton_79Pressed()));
    connect(ui->radioButton_80, SIGNAL(pressed()), this, SLOT(onRadioButton_80Pressed()));


    // L2P1
    connect(ui->radioButton_81, SIGNAL(pressed()), this, SLOT(onRadioButton_81Pressed()));
    connect(ui->radioButton_82, SIGNAL(pressed()), this, SLOT(onRadioButton_82Pressed()));
    connect(ui->radioButton_83, SIGNAL(pressed()), this, SLOT(onRadioButton_83Pressed()));
    connect(ui->radioButton_84, SIGNAL(pressed()), this, SLOT(onRadioButton_84Pressed()));
    connect(ui->radioButton_85, SIGNAL(pressed()), this, SLOT(onRadioButton_85Pressed()));
    connect(ui->radioButton_86, SIGNAL(pressed()), this, SLOT(onRadioButton_86Pressed()));
    connect(ui->radioButton_87, SIGNAL(pressed()), this, SLOT(onRadioButton_87Pressed()));
    connect(ui->radioButton_88, SIGNAL(pressed()), this, SLOT(onRadioButton_88Pressed()));
    connect(ui->radioButton_89, SIGNAL(pressed()), this, SLOT(onRadioButton_89Pressed()));
    connect(ui->radioButton_90, SIGNAL(pressed()), this, SLOT(onRadioButton_90Pressed()));
    connect(ui->radioButton_91, SIGNAL(pressed()), this, SLOT(onRadioButton_91Pressed()));
    connect(ui->radioButton_92, SIGNAL(pressed()), this, SLOT(onRadioButton_92Pressed()));


    // L2P2
    connect(ui->radioButton_93, SIGNAL(pressed()), this, SLOT(onRadioButton_93Pressed()));
    connect(ui->radioButton_94, SIGNAL(pressed()), this, SLOT(onRadioButton_94Pressed()));
    connect(ui->radioButton_95, SIGNAL(pressed()), this, SLOT(onRadioButton_95Pressed()));
    connect(ui->radioButton_96, SIGNAL(pressed()), this, SLOT(onRadioButton_96Pressed()));
    connect(ui->radioButton_97, SIGNAL(pressed()), this, SLOT(onRadioButton_97Pressed()));
    connect(ui->radioButton_98, SIGNAL(pressed()), this, SLOT(onRadioButton_98Pressed()));
    connect(ui->radioButton_99, SIGNAL(pressed()), this, SLOT(onRadioButton_99Pressed()));
    connect(ui->radioButton_100, SIGNAL(pressed()), this, SLOT(onRadioButton_100Pressed()));
    connect(ui->radioButton_101, SIGNAL(pressed()), this, SLOT(onRadioButton_101Pressed()));
    connect(ui->radioButton_102, SIGNAL(pressed()), this, SLOT(onRadioButton_102Pressed()));
    connect(ui->radioButton_103, SIGNAL(pressed()), this, SLOT(onRadioButton_103Pressed()));
    connect(ui->radioButton_104, SIGNAL(pressed()), this, SLOT(onRadioButton_104Pressed()));

  
    // L2P3
    connect(ui->radioButton_105, SIGNAL(pressed()), this, SLOT(onRadioButton_105Pressed()));
    connect(ui->radioButton_106, SIGNAL(pressed()), this, SLOT(onRadioButton_106Pressed()));
    connect(ui->radioButton_107, SIGNAL(pressed()), this, SLOT(onRadioButton_107Pressed()));
    connect(ui->radioButton_108, SIGNAL(pressed()), this, SLOT(onRadioButton_108Pressed()));
    connect(ui->radioButton_109, SIGNAL(pressed()), this, SLOT(onRadioButton_109Pressed()));
    connect(ui->radioButton_110, SIGNAL(pressed()), this, SLOT(onRadioButton_110Pressed()));
    connect(ui->radioButton_111, SIGNAL(pressed()), this, SLOT(onRadioButton_111Pressed()));
    connect(ui->radioButton_112, SIGNAL(pressed()), this, SLOT(onRadioButton_112Pressed()));
    connect(ui->radioButton_113, SIGNAL(pressed()), this, SLOT(onRadioButton_113Pressed()));
    connect(ui->radioButton_114, SIGNAL(pressed()), this, SLOT(onRadioButton_114Pressed()));
    connect(ui->radioButton_115, SIGNAL(pressed()), this, SLOT(onRadioButton_115Pressed()));
    connect(ui->radioButton_116, SIGNAL(pressed()), this, SLOT(onRadioButton_116Pressed()));


    // L3P1
    connect(ui->radioButton_117, SIGNAL(pressed()), this, SLOT(onRadioButton_117Pressed()));
    connect(ui->radioButton_118, SIGNAL(pressed()), this, SLOT(onRadioButton_118Pressed()));
    connect(ui->radioButton_119, SIGNAL(pressed()), this, SLOT(onRadioButton_119Pressed()));
    connect(ui->radioButton_120, SIGNAL(pressed()), this, SLOT(onRadioButton_120Pressed()));
    connect(ui->radioButton_121, SIGNAL(pressed()), this, SLOT(onRadioButton_121Pressed()));
    connect(ui->radioButton_122, SIGNAL(pressed()), this, SLOT(onRadioButton_122Pressed()));
    connect(ui->radioButton_123, SIGNAL(pressed()), this, SLOT(onRadioButton_123Pressed()));
    connect(ui->radioButton_124, SIGNAL(pressed()), this, SLOT(onRadioButton_124Pressed()));
    connect(ui->radioButton_125, SIGNAL(pressed()), this, SLOT(onRadioButton_125Pressed()));
    connect(ui->radioButton_126, SIGNAL(pressed()), this, SLOT(onRadioButton_126Pressed()));
    connect(ui->radioButton_127, SIGNAL(pressed()), this, SLOT(onRadioButton_127Pressed()));
    connect(ui->radioButton_128, SIGNAL(pressed()), this, SLOT(onRadioButton_128Pressed()));


    // L3P2
    connect(ui->radioButton_129, SIGNAL(pressed()), this, SLOT(onRadioButton_129Pressed()));
    connect(ui->radioButton_130, SIGNAL(pressed()), this, SLOT(onRadioButton_130Pressed()));
    connect(ui->radioButton_131, SIGNAL(pressed()), this, SLOT(onRadioButton_131Pressed()));
    connect(ui->radioButton_132, SIGNAL(pressed()), this, SLOT(onRadioButton_132Pressed()));
    connect(ui->radioButton_133, SIGNAL(pressed()), this, SLOT(onRadioButton_133Pressed()));
    connect(ui->radioButton_134, SIGNAL(pressed()), this, SLOT(onRadioButton_134Pressed()));
    connect(ui->radioButton_135, SIGNAL(pressed()), this, SLOT(onRadioButton_135Pressed()));
    connect(ui->radioButton_136, SIGNAL(pressed()), this, SLOT(onRadioButton_136Pressed()));
    connect(ui->radioButton_137, SIGNAL(pressed()), this, SLOT(onRadioButton_137Pressed()));
    connect(ui->radioButton_138, SIGNAL(pressed()), this, SLOT(onRadioButton_138Pressed()));
    connect(ui->radioButton_139, SIGNAL(pressed()), this, SLOT(onRadioButton_139Pressed()));
    connect(ui->radioButton_140, SIGNAL(pressed()), this, SLOT(onRadioButton_140Pressed()));


    // L3P3
    connect(ui->radioButton_141, SIGNAL(pressed()), this, SLOT(onRadioButton_141Pressed()));
    connect(ui->radioButton_142, SIGNAL(pressed()), this, SLOT(onRadioButton_142Pressed()));
    connect(ui->radioButton_143, SIGNAL(pressed()), this, SLOT(onRadioButton_143Pressed()));
    connect(ui->radioButton_144, SIGNAL(pressed()), this, SLOT(onRadioButton_144Pressed()));
    connect(ui->radioButton_145, SIGNAL(pressed()), this, SLOT(onRadioButton_145Pressed()));
    connect(ui->radioButton_146, SIGNAL(pressed()), this, SLOT(onRadioButton_146Pressed()));
    connect(ui->radioButton_147, SIGNAL(pressed()), this, SLOT(onRadioButton_147Pressed()));
    connect(ui->radioButton_148, SIGNAL(pressed()), this, SLOT(onRadioButton_148Pressed()));
    connect(ui->radioButton_149, SIGNAL(pressed()), this, SLOT(onRadioButton_149Pressed()));
    connect(ui->radioButton_150, SIGNAL(pressed()), this, SLOT(onRadioButton_150Pressed()));
    connect(ui->radioButton_151, SIGNAL(pressed()), this, SLOT(onRadioButton_151Pressed()));
    connect(ui->radioButton_152, SIGNAL(pressed()), this, SLOT(onRadioButton_152Pressed()));

   
    // L4P1
    connect(ui->radioButton_153, SIGNAL(pressed()), this, SLOT(onRadioButton_153Pressed()));
    connect(ui->radioButton_154, SIGNAL(pressed()), this, SLOT(onRadioButton_154Pressed()));
    connect(ui->radioButton_155, SIGNAL(pressed()), this, SLOT(onRadioButton_155Pressed()));
    connect(ui->radioButton_156, SIGNAL(pressed()), this, SLOT(onRadioButton_156Pressed()));
    connect(ui->radioButton_157, SIGNAL(pressed()), this, SLOT(onRadioButton_157Pressed()));
    connect(ui->radioButton_158, SIGNAL(pressed()), this, SLOT(onRadioButton_158Pressed()));
    connect(ui->radioButton_159, SIGNAL(pressed()), this, SLOT(onRadioButton_159Pressed()));
    connect(ui->radioButton_160, SIGNAL(pressed()), this, SLOT(onRadioButton_160Pressed()));
    connect(ui->radioButton_161, SIGNAL(pressed()), this, SLOT(onRadioButton_161Pressed()));
    connect(ui->radioButton_162, SIGNAL(pressed()), this, SLOT(onRadioButton_162Pressed()));
    connect(ui->radioButton_163, SIGNAL(pressed()), this, SLOT(onRadioButton_163Pressed()));
    connect(ui->radioButton_164, SIGNAL(pressed()), this, SLOT(onRadioButton_164Pressed()));


    // L4P2
    connect(ui->radioButton_165, SIGNAL(pressed()), this, SLOT(onRadioButton_165Pressed()));
    connect(ui->radioButton_166, SIGNAL(pressed()), this, SLOT(onRadioButton_166Pressed()));
    connect(ui->radioButton_167, SIGNAL(pressed()), this, SLOT(onRadioButton_167Pressed()));
    connect(ui->radioButton_168, SIGNAL(pressed()), this, SLOT(onRadioButton_168Pressed()));
    connect(ui->radioButton_169, SIGNAL(pressed()), this, SLOT(onRadioButton_169Pressed()));
    connect(ui->radioButton_170, SIGNAL(pressed()), this, SLOT(onRadioButton_170Pressed()));
    connect(ui->radioButton_171, SIGNAL(pressed()), this, SLOT(onRadioButton_171Pressed()));
    connect(ui->radioButton_172, SIGNAL(pressed()), this, SLOT(onRadioButton_172Pressed()));
    connect(ui->radioButton_173, SIGNAL(pressed()), this, SLOT(onRadioButton_173Pressed()));
    connect(ui->radioButton_174, SIGNAL(pressed()), this, SLOT(onRadioButton_174Pressed()));
    connect(ui->radioButton_175, SIGNAL(pressed()), this, SLOT(onRadioButton_175Pressed()));
    connect(ui->radioButton_176, SIGNAL(pressed()), this, SLOT(onRadioButton_176Pressed()));


    // L4P3
    connect(ui->radioButton_177, SIGNAL(pressed()), this, SLOT(onRadioButton_177Pressed()));
    connect(ui->radioButton_178, SIGNAL(pressed()), this, SLOT(onRadioButton_178Pressed()));
    connect(ui->radioButton_179, SIGNAL(pressed()), this, SLOT(onRadioButton_179Pressed()));
    connect(ui->radioButton_180, SIGNAL(pressed()), this, SLOT(onRadioButton_180Pressed()));
    connect(ui->radioButton_194, SIGNAL(pressed()), this, SLOT(onRadioButton_194Pressed()));
    connect(ui->radioButton_195, SIGNAL(pressed()), this, SLOT(onRadioButton_195Pressed()));
    connect(ui->radioButton_196, SIGNAL(pressed()), this, SLOT(onRadioButton_196Pressed()));
    connect(ui->radioButton_197, SIGNAL(pressed()), this, SLOT(onRadioButton_197Pressed()));
    connect(ui->radioButton_198, SIGNAL(pressed()), this, SLOT(onRadioButton_198Pressed()));
    connect(ui->radioButton_199, SIGNAL(pressed()), this, SLOT(onRadioButton_199Pressed()));
    connect(ui->radioButton_200, SIGNAL(pressed()), this, SLOT(onRadioButton_200Pressed()));
    connect(ui->radioButton_201, SIGNAL(pressed()), this, SLOT(onRadioButton_201Pressed()));


    // L5P1
    connect(ui->radioButton_202, SIGNAL(pressed()), this, SLOT(onRadioButton_202Pressed()));
    connect(ui->radioButton_203, SIGNAL(pressed()), this, SLOT(onRadioButton_203Pressed()));
    connect(ui->radioButton_204, SIGNAL(pressed()), this, SLOT(onRadioButton_204Pressed()));
    connect(ui->radioButton_205, SIGNAL(pressed()), this, SLOT(onRadioButton_205Pressed()));
    connect(ui->radioButton_206, SIGNAL(pressed()), this, SLOT(onRadioButton_206Pressed()));
    connect(ui->radioButton_207, SIGNAL(pressed()), this, SLOT(onRadioButton_207Pressed()));
    connect(ui->radioButton_208, SIGNAL(pressed()), this, SLOT(onRadioButton_208Pressed()));
    connect(ui->radioButton_209, SIGNAL(pressed()), this, SLOT(onRadioButton_209Pressed()));
    connect(ui->radioButton_210, SIGNAL(pressed()), this, SLOT(onRadioButton_210Pressed()));
    connect(ui->radioButton_211, SIGNAL(pressed()), this, SLOT(onRadioButton_211Pressed()));
    connect(ui->radioButton_212, SIGNAL(pressed()), this, SLOT(onRadioButton_212Pressed()));
    connect(ui->radioButton_213, SIGNAL(pressed()), this, SLOT(onRadioButton_213Pressed()));


    // L5P2
    connect(ui->radioButton_214, SIGNAL(pressed()), this, SLOT(onRadioButton_214Pressed()));
    connect(ui->radioButton_215, SIGNAL(pressed()), this, SLOT(onRadioButton_215Pressed()));
    connect(ui->radioButton_217, SIGNAL(pressed()), this, SLOT(onRadioButton_216Pressed()));
    connect(ui->radioButton_216, SIGNAL(pressed()), this, SLOT(onRadioButton_217Pressed()));
    connect(ui->radioButton_218, SIGNAL(pressed()), this, SLOT(onRadioButton_218Pressed()));
    connect(ui->radioButton_219, SIGNAL(pressed()), this, SLOT(onRadioButton_219Pressed()));
    connect(ui->radioButton_220, SIGNAL(pressed()), this, SLOT(onRadioButton_220Pressed()));
    connect(ui->radioButton_221, SIGNAL(pressed()), this, SLOT(onRadioButton_221Pressed()));
    connect(ui->radioButton_222, SIGNAL(pressed()), this, SLOT(onRadioButton_222Pressed()));
    connect(ui->radioButton_223, SIGNAL(pressed()), this, SLOT(onRadioButton_223Pressed()));
    connect(ui->radioButton_224, SIGNAL(pressed()), this, SLOT(onRadioButton_224Pressed()));
    connect(ui->radioButton_225, SIGNAL(pressed()), this, SLOT(onRadioButton_225Pressed()));


    // L5P3
    connect(ui->radioButton_226, SIGNAL(pressed()), this, SLOT(onRadioButton_226Pressed()));
    connect(ui->radioButton_227, SIGNAL(pressed()), this, SLOT(onRadioButton_227Pressed()));
    connect(ui->radioButton_228, SIGNAL(pressed()), this, SLOT(onRadioButton_228Pressed()));
    connect(ui->radioButton_229, SIGNAL(pressed()), this, SLOT(onRadioButton_229Pressed()));
    connect(ui->radioButton_230, SIGNAL(pressed()), this, SLOT(onRadioButton_230Pressed()));
    connect(ui->radioButton_231, SIGNAL(pressed()), this, SLOT(onRadioButton_231Pressed()));
    connect(ui->radioButton_232, SIGNAL(pressed()), this, SLOT(onRadioButton_232Pressed()));
    connect(ui->radioButton_233, SIGNAL(pressed()), this, SLOT(onRadioButton_233Pressed()));
    connect(ui->radioButton_234, SIGNAL(pressed()), this, SLOT(onRadioButton_234Pressed()));
    connect(ui->radioButton_235, SIGNAL(pressed()), this, SLOT(onRadioButton_235Pressed()));
    connect(ui->radioButton_236, SIGNAL(pressed()), this, SLOT(onRadioButton_236Pressed()));
    connect(ui->radioButton_237, SIGNAL(pressed()), this, SLOT(onRadioButton_237Pressed()));


    // L6P1
    connect(ui->radioButton_238, SIGNAL(pressed()), this, SLOT(onRadioButton_238Pressed()));
    connect(ui->radioButton_239, SIGNAL(pressed()), this, SLOT(onRadioButton_239Pressed()));
    connect(ui->radioButton_240, SIGNAL(pressed()), this, SLOT(onRadioButton_240Pressed()));
    connect(ui->radioButton_241, SIGNAL(pressed()), this, SLOT(onRadioButton_241Pressed()));
    connect(ui->radioButton_242, SIGNAL(pressed()), this, SLOT(onRadioButton_242Pressed()));
    connect(ui->radioButton_243, SIGNAL(pressed()), this, SLOT(onRadioButton_243Pressed()));
    connect(ui->radioButton_244, SIGNAL(pressed()), this, SLOT(onRadioButton_244Pressed()));
    connect(ui->radioButton_245, SIGNAL(pressed()), this, SLOT(onRadioButton_245Pressed()));
    connect(ui->radioButton_246, SIGNAL(pressed()), this, SLOT(onRadioButton_246Pressed()));
    connect(ui->radioButton_247, SIGNAL(pressed()), this, SLOT(onRadioButton_247Pressed()));
    connect(ui->radioButton_248, SIGNAL(pressed()), this, SLOT(onRadioButton_248Pressed()));
    connect(ui->radioButton_249, SIGNAL(pressed()), this, SLOT(onRadioButton_249Pressed()));


    // L6P2
    connect(ui->radioButton_250, SIGNAL(pressed()), this, SLOT(onRadioButton_250Pressed()));
    connect(ui->radioButton_251, SIGNAL(pressed()), this, SLOT(onRadioButton_251Pressed()));
    connect(ui->radioButton_252, SIGNAL(pressed()), this, SLOT(onRadioButton_252Pressed()));
    connect(ui->radioButton_253, SIGNAL(pressed()), this, SLOT(onRadioButton_253Pressed()));
    connect(ui->radioButton_254, SIGNAL(pressed()), this, SLOT(onRadioButton_254Pressed()));
    connect(ui->radioButton_255, SIGNAL(pressed()), this, SLOT(onRadioButton_255Pressed()));
    connect(ui->radioButton_256, SIGNAL(pressed()), this, SLOT(onRadioButton_256Pressed()));
    connect(ui->radioButton_257, SIGNAL(pressed()), this, SLOT(onRadioButton_257Pressed()));
    connect(ui->radioButton_258, SIGNAL(pressed()), this, SLOT(onRadioButton_258Pressed()));
    connect(ui->radioButton_259, SIGNAL(pressed()), this, SLOT(onRadioButton_259Pressed()));
    connect(ui->radioButton_260, SIGNAL(pressed()), this, SLOT(onRadioButton_260Pressed()));
    connect(ui->radioButton_261, SIGNAL(pressed()), this, SLOT(onRadioButton_261Pressed()));


    // L6P3
    connect(ui->radioButton_262, SIGNAL(pressed()), this, SLOT(onRadioButton_262Pressed()));
    connect(ui->radioButton_263, SIGNAL(pressed()), this, SLOT(onRadioButton_263Pressed()));
    connect(ui->radioButton_264, SIGNAL(pressed()), this, SLOT(onRadioButton_264Pressed()));
    connect(ui->radioButton_265, SIGNAL(pressed()), this, SLOT(onRadioButton_265Pressed()));
    connect(ui->radioButton_266, SIGNAL(pressed()), this, SLOT(onRadioButton_266Pressed()));
    connect(ui->radioButton_267, SIGNAL(pressed()), this, SLOT(onRadioButton_267Pressed()));
    connect(ui->radioButton_268, SIGNAL(pressed()), this, SLOT(onRadioButton_268Pressed()));
    connect(ui->radioButton_269, SIGNAL(pressed()), this, SLOT(onRadioButton_269Pressed()));
    connect(ui->radioButton_270, SIGNAL(pressed()), this, SLOT(onRadioButton_270Pressed()));
    connect(ui->radioButton_271, SIGNAL(pressed()), this, SLOT(onRadioButton_271Pressed()));
    connect(ui->radioButton_272, SIGNAL(pressed()), this, SLOT(onRadioButton_272Pressed()));
    connect(ui->radioButton_273, SIGNAL(pressed()), this, SLOT(onRadioButton_273Pressed()));


    // data type in modbus request groupbox
    connect(ui->radioButton_181, SIGNAL(toggled(bool)), this, SLOT(onFloatButtonPressed(bool)));
    connect(ui->radioButton_182, SIGNAL(toggled(bool)), this, SLOT(onIntegerButtonPressed(bool)));
    connect(ui->radioButton_183, SIGNAL(toggled(bool)), this, SLOT(onCoilButtonPressed(bool)));

    // select R/W mode
    connect(ui->radioButton_187, SIGNAL(toggled(bool)), this, SLOT(onReadButtonPressed(bool)));
    connect(ui->radioButton_186, SIGNAL(toggled(bool)), this, SLOT(onWriteButtonPressed(bool)));
}


void
MainWindow::
connectSerialPort()
{
    connect( ui->groupBox_18, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( ui->groupBox_28, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( ui->groupBox_38, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( ui->groupBox_48, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( ui->groupBox_58, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( ui->groupBox_68, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxChecked(bool)));
    connect( this, SIGNAL(connectionError(const QString&)), this, SLOT(setStatusError(const QString&)));
}


void
MainWindow::
connectActions()
{
    connect( ui->actionAbout_QModBus, SIGNAL( triggered() ),this, SLOT( aboutQModBus() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( enableHexView() ) );
}


void
MainWindow::
connectModbusMonitor()
{
    connect( ui->slaveID, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->startAddr, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->numCoils, SIGNAL( valueChanged( int ) ),this, SLOT( updateRequestPreview() ) );
    connect( ui->functionCode, SIGNAL( currentIndexChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->numCoils, SIGNAL( valueChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->startAddr, SIGNAL( valueChanged( int ) ),this, SLOT( updateRegisterView() ) );
    connect( ui->sendBtn, SIGNAL(pressed()),this, SLOT( onSendButtonPress() ) );
    connect( ui->groupBox_105, SIGNAL( toggled(bool)), this, SLOT( onEquationTableChecked(bool)));
}


void
MainWindow::
connectToolbar()
{
    connect(ui->actionSave, SIGNAL(triggered()),this,SLOT(saveCsvFile()));
    connect(ui->actionOpen, SIGNAL(triggered()),this,SLOT(loadCsvFile()));
}


void
MainWindow::
onEquationTableChecked(bool isTable)
{
    if (!isTable) ui->tableWidget->setRowCount(0);
}


QString
MainWindow::
sendCalibrationRequest(int dataType, modbus_t * serialModbus, int func, int addr, int num, int ret, uint8_t * dest, uint16_t * dest16, bool is16Bit, bool writeAccess, QString funcType)
{
    switch( func )
    {
        case MODBUS_FC_READ_COILS:
            ret = modbus_read_bits( serialModbus, addr, num, dest );
            break;
        case MODBUS_FC_READ_DISCRETE_INPUTS:
            ret = modbus_read_input_bits( serialModbus, addr, num, dest );
            break;
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            ret = modbus_read_registers( serialModbus, addr, num, dest16 );
            is16Bit = true;
            break;
        case MODBUS_FC_READ_INPUT_REGISTERS:
            ret = modbus_read_input_registers(serialModbus, addr, num, dest16 );
            is16Bit = true;
            break;
        case MODBUS_FC_WRITE_SINGLE_COIL:
            //ret = modbus_write_bit( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) ? 1 : 0 );
            ret = modbus_write_bit( serialModbus, addr,ui->radioButton_184->isChecked() ? 1 : 0 );
            writeAccess = true;
            num = 1;
            break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            //ret = modbus_write_register( m_modbus_snipping, addr,ui->regTable->item( 0, DataColumn )->text().toInt(0, 0) );
            ret = modbus_write_register( serialModbus, addr,ui->lineEdit_111->text().toInt(0, 0) );
            writeAccess = true;
            num = 1;
            break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            uint8_t * data = new uint8_t[num];
            for( int i = 0; i < num; ++i ) data[i] = ui->regTable->item( i, DataColumn )->text().toInt(0, 0);
            ret = modbus_write_bits( serialModbus, addr, num, data );
            delete[] data;
            writeAccess = true;
            break;
        }
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            float value;
            QString qvalue = ui->lineEdit_109->text();
            QTextStream floatTextStream(&qvalue);
            floatTextStream >> value;
            quint16 (*reg)[2] = reinterpret_cast<quint16(*)[2]>(&value);
            uint16_t * data = new uint16_t[2];
            data[0] = (*reg)[1];
            data[1] = (*reg)[0];
            ret = modbus_write_registers( serialModbus, addr, 2, data );
            delete[] data;
            writeAccess = true;
            break;
        }
        default:
            break;
    }

    if( ret == num  )
    {
        if( writeAccess )
        {
            m_statusText->setText(tr( "Values successfully sent" ) );
            m_statusInd->setStyleSheet( "background: #0b0;" );
            m_statusTimer->start( 2000 );
        }
        else
        {
            //bool b_hex = is16Bit && ui->checkBoxHexData->checkState() == Qt::Checked;
            QString qs_num;
            QString qs_output = "0x";
            bool ok = false;

            ui->regTable->setRowCount( num );
            for( int i = 0; i < num; ++i )
            {
                int data = is16Bit ? dest16[i] : dest[i];
                QString qs_tmp;

                //QTableWidgetItem * dtItem = new QTableWidgetItem( funcType );
                //QTableWidgetItem * addrItem = new QTableWidgetItem(QString::number( ui->startAddr->value()+i ) );
                //qs_num.sprintf( b_hex ? "0x%04x" : "%d", data);
                qs_num.sprintf("%d", data);
                qs_tmp.sprintf("%04x", data);
                qs_output.append(qs_tmp);

                //QTableWidgetItem * dataItem = new QTableWidgetItem( qs_num );
                //dtItem->setFlags( dtItem->flags() & ~Qt::ItemIsEditable );
                //addrItem->setFlags( addrItem->flags() & ~Qt::ItemIsEditable );
                //dataItem->setFlags( dataItem->flags() & ~Qt::ItemIsEditable );
                //ui->regTable->setItem( i, DataTypeColumn, dtItem );
                //ui->regTable->setItem( i, AddrColumn, addrItem );
                //ui->regTable->setItem( i, DataColumn, dataItem );

                if (ui->radioButton_182->isChecked()) ui->lineEdit_111->setText(QString::number(data));
                else if (ui->radioButton_183->isChecked())
                {
                    (data) ? ui->radioButton_184->setChecked(true) : ui->radioButton_185->setChecked(true);
                }
                if (dataType == FLOAT_R) // FLOAT_READ
                {
                    QByteArray array = QByteArray::fromHex(qs_output.toLatin1());
                    const float d = toFloat(array);
                    return QString::number(d,'f',6);
                }
                else if (dataType == INT_R)  // INT_READ
                {
                    return QString::number(data);
                }
                else if (dataType == COIL_R)  // COIL_READ
                {
                    return (data) ? "1" : "0";
                }
            }
        }
    }
    else
    {
        QString err;

        if( ret < 0 )
        {
            if(
#ifdef WIN32
                    errno == WSAETIMEDOUT ||
#endif
                    errno == EIO
                                                                    )
            {
                err += tr( "I/O error" );
                err += ": ";
                err += tr( "did not receive any data from slave." );
            }
            else
            {
                err += tr( "Protocol error" );
                err += ": ";
                err += tr( "Slave threw exception '" );
                err += modbus_strerror( errno );
                err += tr( "' or function not implemented." );
            }
        }
        else
        {
            err += tr( "Protocol error" );
            err += ": ";
            err += tr( "Number of registers returned does not "
                    "match number of registers requested!" );
        }

        if( err.size() > 0 )
            setStatusError( err );
    }
}


void
MainWindow::
saveCsvFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save Equation"), "",tr("CSV file (*.csv);;All Files (*)"));

    if (fileName.isEmpty()) return;
    QFile file(fileName);
    QTextStream out(&file);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(this, tr("Unable to open file"),file.errorString());
        return;
    }

    for ( int i = 0; i < ui->tableWidget->rowCount(); i++ )
    {
        QString dataStream;

        for (int j=0; j < ui->tableWidget->item(i,6)->text().toInt()+7; j++)
        {
             dataStream.append(ui->tableWidget->item(i,j)->text()+",");
        }

        out << dataStream << endl;
    }

    file.close();
}


void
MainWindow::
loadCsvTemplate()
{
    int line = 0;
    QFile file;
    QString razorTemplatePath = QCoreApplication::applicationDirPath()+"/razor.csv";
    QString eeaTemplatePath = QCoreApplication::applicationDirPath()+"/eea.csv";

    if (ui->radioButton_190->isChecked()) // eea
        file.setFileName(eeaTemplatePath);
    else
        file.setFileName(razorTemplatePath);

    if (!file.open(QIODevice::ReadOnly)) return;

    QTextStream str(&file);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    while (!str.atEnd()) {

        QString s = str.readLine();
        if (s.size() == 0)
        {
            file.close();
            break;
        }
        else {
            line++;
        }

        // split data
        QStringList valueList = s.split(',');

        if (!valueList[0].contains("*"))
        {
            // insert a new row
            ui->tableWidget->insertRow( ui->tableWidget->rowCount() );

            // insert columns
            while (ui->tableWidget->columnCount() < valueList[6].toInt()+7)
            {
                ui->tableWidget->insertColumn(ui->tableWidget->columnCount());
            }

            // fill the data in the talbe cell
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(valueList[0])); // Name
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(valueList[1])); // Slave
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 2, new QTableWidgetItem(valueList[2])); // Address
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(valueList[3])); // Type
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 4, new QTableWidgetItem(valueList[4])); // Scale
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 5, new QTableWidgetItem(valueList[5])); // RW
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 6, new QTableWidgetItem(valueList[6])); // Qty
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 7, new QTableWidgetItem(valueList[7])); // Value

            // enable uploadEquationButton
            ui->startEquationBtn->setEnabled(1);
        }
    }

    // set column width
    ui->tableWidget->setColumnWidth(0,120); // Name
    ui->tableWidget->setColumnWidth(1,30);  // Slave
    ui->tableWidget->setColumnWidth(2,50);  // Address
    ui->tableWidget->setColumnWidth(3,40);  // Type
    ui->tableWidget->setColumnWidth(4,30);  // Scale
    ui->tableWidget->setColumnWidth(5,30);  // RW
    ui->tableWidget->setColumnWidth(6,30);  // Qty

    // close file
    file.close();
}

void
MainWindow::
loadCsvFile()
{
    int line = 0;

    QString fileName = QFileDialog::getOpenFileName( this, tr("Open CSV file"), QDir::currentPath(), tr("CSV files (*.csv)") );
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) return;

    QTextStream str(&file);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    while (!str.atEnd()) {

        QString s = str.readLine();
        if (s.size() == 0)
        {
            file.close();
            break;
        }
        else {
            line++;
        }

        QStringList valueList = s.split(',');
        if (!valueList[0].contains("*"))
        {
            // insert a new row
            ui->tableWidget->insertRow( ui->tableWidget->rowCount() );

            // insert columns
            while (ui->tableWidget->columnCount() < valueList[6].toInt()+7)
            {
                ui->tableWidget->insertColumn(ui->tableWidget->columnCount());
            }

            // fill the data in the talbe cell
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 0, new QTableWidgetItem(valueList[0])); // Name
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 1, new QTableWidgetItem(valueList[1])); // Slave
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 2, new QTableWidgetItem(valueList[2])); // Address
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 3, new QTableWidgetItem(valueList[3])); // Type
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 4, new QTableWidgetItem(valueList[4])); // Scale
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 5, new QTableWidgetItem(valueList[5])); // RW
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 6, new QTableWidgetItem(valueList[6])); // Qty
            ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, 7, new QTableWidgetItem(valueList[7])); // Value

            // fill the value list
            for (int j = 0; j < valueList[6].toInt(); j++)
            {
                QString cellData = valueList[7+j];
                if (valueList[3].contains("int")) cellData = cellData.mid(0, cellData.indexOf("."));

                ui->tableWidget->setItem( ui->tableWidget->rowCount()-1, j+7, new QTableWidgetItem(cellData));
            }

            // enable uploadEquationButton
            ui->startEquationBtn->setEnabled(1);
        }
    }

    // set column width
    ui->tableWidget->setColumnWidth(0,120); // Name
    ui->tableWidget->setColumnWidth(1,30);  // Slave
    ui->tableWidget->setColumnWidth(2,50);  // Address
    ui->tableWidget->setColumnWidth(3,40);  // Type
    ui->tableWidget->setColumnWidth(4,30);  // Scale
    ui->tableWidget->setColumnWidth(5,30);  // RW
    ui->tableWidget->setColumnWidth(6,30);  // Qty

    // close file
    file.close();
}

void
MainWindow::
onEquationButtonPressed()
{
    ui->startEquationBtn->setEnabled(false);
    ui->startEquationBtn->setText( tr("Loading") );

    if (ui->radioButton_188->isChecked())
    {
        if( m_pollTimer->isActive() )
        {
            m_pollTimer->stop();
            ui->startEquationBtn->setText( tr("Loading") );
        }
        else
        {
            // if polling requested then enable timer
            if( m_poll )
            {
                m_pollTimer->start( 1000 );
                ui->sendBtn->setText( tr("Loading") );
            }

            onUploadEquation();
        }
    }
    else
    {
        if( m_pollTimer->isActive() )
        {
            m_pollTimer->stop();
            ui->startEquationBtn->setText( tr("Loading") );
        }
        else
        {
            // if polling requested then enable timer
            if( m_poll )
            {
                m_pollTimer->start( 1000 );
                ui->sendBtn->setText( tr("Loading") );
            }
        
            ui->tableWidget->clearContents();
            ui->tableWidget->setRowCount(0);

            onDownloadEquation();
        }
    }

    ui->startEquationBtn->setText(tr("Start"));
    ui->startEquationBtn->setEnabled(true);
}


void
MainWindow::
onDownloadButtonChecked(bool isChecked)
{
    if (isChecked)
    {
        ui->startEquationBtn->setEnabled(true);
    }
    else {
        ui->startEquationBtn->setEnabled(true);
    }
}


void
MainWindow::
onDownloadEquation()
{
    int value = 0;
    int rangeMax = 0;

    ui->slaveID->setValue(1);                           // set slave ID
    ui->radioButton_187->setChecked(true);              // read mode
    ui->startEquationBtn->setEnabled(false);

    // load empty equation file
    loadCsvTemplate();

    /// get rangeMax of progressDialog
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) rangeMax+=ui->tableWidget->item(i,6)->text().toInt();

    QProgressDialog progress("Downloading...", "Abort", 0, rangeMax, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.setAutoReset(true);

    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
         int regAddr = ui->tableWidget->item(i,2)->text().toInt();

         if (ui->tableWidget->item(i,3)->text().contains("float"))
         {
             ui->numCoils->setValue(2);                  // 2 bytes
             ui->radioButton_181->setChecked(TRUE);      // float type
             ui->functionCode->setCurrentIndex(3);       // function code
             for (int x=0; x < ui->tableWidget->item(i,6)->text().toInt(); x++)
             {
                if (progress.wasCanceled()) return;
                if (ui->tableWidget->item(i,6)->text().toInt() > 1) progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\" "+QString::number(x+1));
                else progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
                progress.setValue(value++);

                 ui->startAddr->setValue(regAddr);       // set address
                 onSendButtonPress();                    // send
                 delay();
                 regAddr = regAddr+2;                    // update reg address
                 ui->tableWidget->setItem( i, x+7, new QTableWidgetItem(ui->lineEdit_109->text()));
             }
         }
         else if (ui->tableWidget->item(i,3)->text().contains("int"))
         {
            if (progress.wasCanceled()) return;
            progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
            progress.setValue(value++);

             ui->numCoils->setValue(1);                  // 1 byte
             ui->radioButton_182->setChecked(TRUE);      // int type
             ui->functionCode->setCurrentIndex(3);       // function code
             ui->startAddr->setValue(regAddr);           // address
             onSendButtonPress();                        // send
             delay();
             ui->tableWidget->setItem( i, 7, new QTableWidgetItem(ui->lineEdit_111->text()));
         }
         else
         {
            if (progress.wasCanceled()) return;
            progress.setLabelText("Downloading \""+ui->tableWidget->item(i,0)->text()+"\"");
            progress.setValue(value++);

             ui->numCoils->setValue(1);                  // 1 byte
             ui->radioButton_183->setChecked(TRUE);      // coil type
             ui->functionCode->setCurrentIndex(0);       // function code
             ui->startAddr->setValue(regAddr);           // address
             onSendButtonPress();                        // send
             delay();
         }
     }
}



void
MainWindow::
onUploadEquation()
{
    int value = 0;
    int rangeMax = 0;
    bool isReinit = false;
    QMessageBox msgBox;
	isModbusTransmissionFailed = false;

    /// get rangeMax of progressDialog
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) rangeMax+=ui->tableWidget->item(i,6)->text().toInt();
    
    msgBox.setText("You can reinitialize existing registers and coils.");
    msgBox.setInformativeText("Do you want to reinitialize registers and coils?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes:
            isReinit = true;
            break;
        case QMessageBox::No:
            isReinit = false;
            break;
        case QMessageBox::Cancel:
        default: return;
    }
        
    ui->slaveID->setValue(1);                           // set slave ID
    ui->radioButton_186->setChecked(true);              // write mode    

    QProgressDialog progress("Uploading...", "Abort", 0, rangeMax, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setAutoClose(true);
    progress.setAutoReset(true);

    /// unlock fct default regs & coils (999)
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function type
    ui->startAddr->setValue(999);                   // address
    ui->radioButton_184->setChecked(true);          // set value
    if (progress.wasCanceled()) return;
    progress.setValue(0);
    progress.setLabelText("Unlocking factory registers....");
    onSendButtonPress();
    delay();
	if (isModbusTransmissionFailed) 
	{
		isModbusTransmissionFailed = false;
		msgBox.setText("Modbus Transmission Failed.");
    	msgBox.setInformativeText("Do you want to continue with next item?");
    	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    	msgBox.setDefaultButton(QMessageBox::No);
    	int ret = msgBox.exec();
    	switch (ret) {
        	case QMessageBox::Yes:
            	break;
        	case QMessageBox::No:
        	default: return;
    	}

	}

    if (isReinit)
    {
        ui->numCoils->setValue(1);                      // set byte count 1
        ui->radioButton_183->setChecked(TRUE);          // set type coil 
        ui->functionCode->setCurrentIndex(4);           // set function type
        ui->radioButton_185->setChecked(true);          // set coils unlocked 

        ui->startAddr->setValue(25);                    // set address 25
        if (progress.wasCanceled()) return;
        progress.setLabelText("Reinitializing registers....");
        progress.setValue(0);
        onSendButtonPress();
        delay();
		if (isModbusTransmissionFailed) 
		{
			isModbusTransmissionFailed = false;
			msgBox.setText("Modbus Transmission Failed.");
    		msgBox.setInformativeText("Do you want to continue with next item?");
    		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    		msgBox.setDefaultButton(QMessageBox::No);
    		int ret = msgBox.exec();
    		switch (ret) {
        		case QMessageBox::Yes: break;
        		case QMessageBox::No:
        		default: return;
    		}

		}
        ui->startAddr->setValue(26);                    // set address 26
        if (progress.wasCanceled()) return;
        progress.setValue(0);
        onSendButtonPress();
        delay(8);                                       // need extra time to restart
		if (isModbusTransmissionFailed) 
		{
			isModbusTransmissionFailed = false;
			msgBox.setText("Modbus Transmission Failed.");
    		msgBox.setInformativeText("Do you want to continue with next item?");
    		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    		msgBox.setDefaultButton(QMessageBox::No);
    		int ret = msgBox.exec();
    		switch (ret) {
        		case QMessageBox::Yes: break;
        		case QMessageBox::No:
        		default: return;
    		}

		}	
        /// unlock fct default regs & coils (999)
        ui->numCoils->setValue(1);                      // 1 byte
        ui->radioButton_183->setChecked(TRUE);          // coil type
        ui->functionCode->setCurrentIndex(4);           // function type
        ui->startAddr->setValue(999);                   // address
        ui->radioButton_184->setChecked(true);          // set value
        if (progress.wasCanceled()) return;
        progress.setValue(0);
        progress.setLabelText("Unlocking factory registers....");
        onSendButtonPress();
        delay();
		if (isModbusTransmissionFailed) 
		{
			isModbusTransmissionFailed = false;
			msgBox.setText("Modbus Transmission Failed.");
    		msgBox.setInformativeText("Do you want to continue with next item?");
    		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    		msgBox.setDefaultButton(QMessageBox::No);
    		int ret = msgBox.exec();
    		switch (ret) {
        		case QMessageBox::Yes: break;
        		case QMessageBox::No:
        		default: return;
    		}
		}
   }

   for (int i = 0; i < ui->tableWidget->rowCount(); i++)
   {
        int regAddr = ui->tableWidget->item(i,2)->text().toInt();
        if (ui->tableWidget->item(i,3)->text().contains("float"))
        {
            ui->numCoils->setValue(2);                  // 2 bytes
            ui->radioButton_181->setChecked(TRUE);      // float type
            ui->functionCode->setCurrentIndex(7);       // function code
            for (int x=0; x < ui->tableWidget->item(i,6)->text().toInt(); x++)
            {
                QString val = ui->tableWidget->item(i,7+x)->text();
                ui->startAddr->setValue(regAddr);       // set address
                ui->lineEdit_109->setText(val);         // set value
                if (progress.wasCanceled()) return;
                if (ui->tableWidget->item(i,6)->text().toInt() > 1) progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"["+QString::number(x+1)+"]"+"\""+","+" \""+val+"\"");
                else progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \""+val+"\"");
                progress.setValue(value++);
                onSendButtonPress();                    // send
                regAddr += 2;                           // update reg address
                delay();
				if (isModbusTransmissionFailed) 
				{
					isModbusTransmissionFailed = false;
					if (ui->tableWidget->item(i,6)->text().toInt() > 1) msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text()+"["+QString::number(x+1)+"]");
					else msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
    				msgBox.setInformativeText("Do you want to continue with next item?");
    				msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    				msgBox.setDefaultButton(QMessageBox::No);
    				int ret = msgBox.exec();
    				switch (ret) {
        				case QMessageBox::Yes: break;
        				case QMessageBox::No:
        				default: return;
    				}
				}
            }
        }
        else if (ui->tableWidget->item(i,3)->text().contains("int"))
        {
            QString val = ui->tableWidget->item(i,7)->text();
            ui->numCoils->setValue(1);                  // 1 byte
            ui->radioButton_182->setChecked(TRUE);      // int type
            ui->functionCode->setCurrentIndex(5);       // function code
            ui->lineEdit_111->setText(val);             // set value
            ui->startAddr->setValue(regAddr);           // address
            if (progress.wasCanceled()) return;
            progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \""+val+"\"");
            progress.setValue(value++);
            onSendButtonPress();                        // send
            delay();
			if (isModbusTransmissionFailed) 
			{
				isModbusTransmissionFailed = false;
				msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
    			msgBox.setInformativeText("Do you want to continue with next item?");
    			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    			msgBox.setDefaultButton(QMessageBox::No);
    			int ret = msgBox.exec();
    			switch (ret) {
        			case QMessageBox::Yes: break;
        			case QMessageBox::No:
        			default: return;
    			}
			}
        }
        else
        {
            ui->numCoils->setValue(1);                  // 1 byte
            ui->radioButton_183->setChecked(TRUE);      // coil type
            ui->functionCode->setCurrentIndex(4);       // function code
            ui->startAddr->setValue(regAddr);           // address
            if (ui->tableWidget->item(i,7)->text().toInt() == 1)
            {
                ui->radioButton_184->setChecked(true);  // TRUE
                progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \"1\"");
            }
            else 
            {
                ui->radioButton_185->setChecked(true);  // FALSE
                progress.setLabelText("Uploading \""+ui->tableWidget->item(i,0)->text()+"\""+","+" \"0\"");
            }
            if (progress.wasCanceled()) return;
            progress.setValue(value++);
            onSendButtonPress();                        // send
            delay();
			if (isModbusTransmissionFailed) 
			{
				isModbusTransmissionFailed = false;
				msgBox.setText("Modbus Transmission Failed: "+ui->tableWidget->item(i,0)->text());
    			msgBox.setInformativeText("Do you want to continue with next item?");
    			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    			msgBox.setDefaultButton(QMessageBox::No);
    			int ret = msgBox.exec();
    			switch (ret) {
        			case QMessageBox::Yes: break;
        			case QMessageBox::No:
        			default: return;
    			}
			}
        }
    }

    /// update factory default values
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_184->setChecked(true);          // set value

    /// unlock factory default registers
    ui->startAddr->setValue(999);                   // address 999
    onSendButtonPress();
    delay();

    /// update factory default registers
    ui->startAddr->setValue(9999);                  // address 99999
    onSendButtonPress();
    delay();
}

void
MainWindow::
onUnlockFactoryDefaultBtnPressed()
{
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_184->setChecked(true);          // set value

    /// unlock factory default registers
    ui->startAddr->setValue(999);                   // address 999
    onSendButtonPress();
    delay();
}

void
MainWindow::
onLockFactoryDefaultBtnPressed()
{
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_185->setChecked(true);          // set value

    /// unlock factory default registers
    ui->startAddr->setValue(999);                   // address 999
    onSendButtonPress();
    delay();
}

void
MainWindow::
onUpdateFactoryDefaultPressed()
{
    if ( ui->radioButton_192->isChecked()) return;

    QMessageBox msgBox;

    msgBox.setText("Factory default values will be permanently changed.");
    msgBox.setInformativeText("Are you sure you want to do this?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    switch (ret) {
        case QMessageBox::Yes: break;
        case QMessageBox::No:
        default: return;
    }
 
    ui->numCoils->setValue(1);                      // 1 byte
    ui->radioButton_183->setChecked(TRUE);          // coil type
    ui->functionCode->setCurrentIndex(4);           // function code
    ui->radioButton_184->setChecked(true);          // set value

    /// update factory default registers
    ui->startAddr->setValue(9999);                  // address 99999
    onSendButtonPress();
    delay();
}


void
MainWindow::
connectProfiler()
{
    connect(ui->radioButton_193, SIGNAL(pressed()), this, SLOT(onUnlockFactoryDefaultBtnPressed()));
    connect(ui->radioButton_192, SIGNAL(pressed()), this, SLOT(onLockFactoryDefaultBtnPressed()));
    connect(ui->pushButton_2, SIGNAL(pressed()), this, SLOT(onUpdateFactoryDefaultPressed()));
    connect(ui->startEquationBtn, SIGNAL(pressed()), this, SLOT(onEquationButtonPressed()));
    connect(ui->radioButton_189, SIGNAL(toggled(bool)), this, SLOT(onDownloadButtonChecked(bool)));
}


void
MainWindow::
initializeGauges()
{
    initializeFrequencyGauge();
    updateFrequencyGauge();

    initializeTemperatureGauge();
    updateTemperatureGauge();

    initializeDensityGauge();
    updateDensityGauge();

    initializeRPGauge();
    updateRPGauge();
}

void
MainWindow::
setupModbusPorts()
{
    setupModbusPort();
    setupModbusPort_2();
    setupModbusPort_3();
    setupModbusPort_4();
    setupModbusPort_5();
    setupModbusPort_6();
}

int
MainWindow::
setupModbusPort()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox->disconnect();
    ui->comboBox->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
        ui->comboBox->addItem( port.friendName );

        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox->setCurrentIndex( portIndex );
    ui->comboBox_2->setCurrentIndex(0);
    ui->comboBox_3->setCurrentIndex(0);
    ui->comboBox_4->setCurrentIndex(0);
    ui->comboBox_5->setCurrentIndex(0);

    connect( ui->comboBox, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_2, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_3, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_4, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );
    connect( ui->comboBox_5, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort( int ) ) );

    changeSerialPort( portIndex );
    return portIndex;
}

int
MainWindow::
setupModbusPort_2()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox_6->disconnect();
    ui->comboBox_6->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
#ifdef Q_OS_WIN
        ui->comboBox_6->addItem( port.friendName );
#else
        ui->comboBox_6->addItem( port.physName );
#endif
        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox_6->setCurrentIndex( portIndex );
    ui->comboBox_7->setCurrentIndex(0);
    ui->comboBox_8->setCurrentIndex(0);
    ui->comboBox_9->setCurrentIndex(0);
    ui->comboBox_10->setCurrentIndex(0);

    connect( ui->comboBox_6, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_2( int ) ) );
    connect( ui->comboBox_7, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_2( int ) ) );
    connect( ui->comboBox_8, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_2( int ) ) );
    connect( ui->comboBox_9, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_2( int ) ) );
    connect( ui->comboBox_10, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_2( int ) ) );

    changeSerialPort_2( portIndex );
    return portIndex;
}

int
MainWindow::
setupModbusPort_3()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox_11->disconnect();
    ui->comboBox_11->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
#ifdef Q_OS_WIN
        ui->comboBox_11->addItem( port.friendName );
#else
        ui->comboBox_11->addItem( port.physName );
#endif
        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox_11->setCurrentIndex( portIndex );
    ui->comboBox_12->setCurrentIndex(0);
    ui->comboBox_13->setCurrentIndex(0);
    ui->comboBox_14->setCurrentIndex(0);
    ui->comboBox_15->setCurrentIndex(0);

    connect( ui->comboBox_11, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_3( int ) ) );
    connect( ui->comboBox_12, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_3( int ) ) );
    connect( ui->comboBox_13, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_3( int ) ) );
    connect( ui->comboBox_14, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_3( int ) ) );
    connect( ui->comboBox_15, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_3( int ) ) );

    changeSerialPort_3( portIndex );
    return portIndex;
}

int
MainWindow::
setupModbusPort_4()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox_16->disconnect();
    ui->comboBox_16->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
#ifdef Q_OS_WIN
        ui->comboBox_16->addItem( port.friendName );
#else
        ui->comboBox_16->addItem( port.physName );
#endif
        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox_16->setCurrentIndex( portIndex );

    ui->comboBox_17->setCurrentIndex(0);
    ui->comboBox_18->setCurrentIndex(0);
    ui->comboBox_19->setCurrentIndex(0);
    ui->comboBox_20->setCurrentIndex(0);

    connect( ui->comboBox_16, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_4( int ) ) );
    connect( ui->comboBox_17, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_4( int ) ) );
    connect( ui->comboBox_18, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_4( int ) ) );
    connect( ui->comboBox_19, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_4( int ) ) );
    connect( ui->comboBox_20, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_4( int ) ) );

    changeSerialPort_4( portIndex );
    return portIndex;
}

int
MainWindow::
setupModbusPort_5()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox_21->disconnect();
    ui->comboBox_21->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
#ifdef Q_OS_WIN
        ui->comboBox_21->addItem( port.friendName );
#else
        ui->comboBox_21->addItem( port.physName );
#endif
        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox_21->setCurrentIndex( portIndex );
    ui->comboBox_22->setCurrentIndex(0);
    ui->comboBox_23->setCurrentIndex(0);
    ui->comboBox_24->setCurrentIndex(0);
    ui->comboBox_25->setCurrentIndex(0);

    connect( ui->comboBox_21, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_5( int ) ) );
    connect( ui->comboBox_22, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_5( int ) ) );
    connect( ui->comboBox_23, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_5( int ) ) );
    connect( ui->comboBox_24, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_5( int ) ) );
    connect( ui->comboBox_25, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_5( int ) ) );

    changeSerialPort_5( portIndex );
    return portIndex;
}

int
MainWindow::
setupModbusPort_6()
{
    QSettings s;

    int portIndex = 0;
    int i = 0;
    ui->comboBox_26->disconnect();
    ui->comboBox_26->clear();
    foreach( QextPortInfo port, QextSerialEnumerator::getPorts() )
    {
#ifdef Q_OS_WIN
        ui->comboBox_26->addItem( port.friendName );
#else
        ui->comboBox_26->addItem( port.physName );
#endif
        if( port.friendName == s.value( "serialinterface" ) )
        {
            portIndex = i;
        }
        ++i;
    }
    ui->comboBox_26->setCurrentIndex( portIndex );
    ui->comboBox_27->setCurrentIndex(0);
    ui->comboBox_28->setCurrentIndex(0);
    ui->comboBox_29->setCurrentIndex(0);
    ui->comboBox_30->setCurrentIndex(0);

    connect( ui->comboBox_26, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_6( int ) ) );
    connect( ui->comboBox_27, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_6( int ) ) );
    connect( ui->comboBox_28, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_6( int ) ) );
    connect( ui->comboBox_29, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_6( int ) ) );
    connect( ui->comboBox_30, SIGNAL( currentIndexChanged( int ) ),this, SLOT( changeSerialPort_6( int ) ) );

    changeSerialPort_6( portIndex );
    return portIndex;
}


//static inline QString embracedString( const QString & s )
//{
    //return s.section( '(', 1 ).section( ')', 0, 0 );
//}


void
MainWindow::
releaseSerialModbus()
{
    if( m_serialModbus )
    {
        modbus_close( m_serialModbus );
        modbus_free( m_serialModbus );
        m_serialModbus = NULL;
        updateTabIcon(0, false);
    }
}

void
MainWindow::
releaseSerialModbus_2()
{
    if( m_serialModbus_2 )
    {
        modbus_close( m_serialModbus_2 );
        modbus_free( m_serialModbus_2 );
        m_serialModbus_2 = NULL;
        updateTabIcon(1, false);
    }
}

void
MainWindow::
releaseSerialModbus_3()
{
    if( m_serialModbus_3 )
    {
        modbus_close( m_serialModbus_3 );
        modbus_free( m_serialModbus_3 );
        m_serialModbus_3 = NULL;
        updateTabIcon(2, false);
    }
}

void
MainWindow::
releaseSerialModbus_4()
{
    if( m_serialModbus_4 )
    {
        modbus_close( m_serialModbus_4);
        modbus_free( m_serialModbus_4 );
        m_serialModbus_4 = NULL;
        updateTabIcon(3, false);
    }
}


void
MainWindow::
releaseSerialModbus_5()
{
    if( m_serialModbus_5 )
    {
        modbus_close( m_serialModbus_5 );
        modbus_free( m_serialModbus_5 );
        m_serialModbus_5 = NULL;
        updateTabIcon(4, false);
    }
}


void
MainWindow::
releaseSerialModbus_6()
{
    if( m_serialModbus_6 )
    {
        modbus_close( m_serialModbus_6 );
        modbus_free( m_serialModbus_6 );
        m_serialModbus_6 = NULL;
        updateTabIcon(5, false);
    }
}

void
MainWindow::
changeSerialPort( int )
{
    const int iface = ui->comboBox->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_2->currentText() );
        settings.setValue( "serialparity", ui->comboBox_3->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_4->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_5->currentText() );
        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_3->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface(port, parity);
        onRtuPortActive(true);
    }
    else emit connectionError( tr( "No serial port found at Loop_1" ) );
}


void
MainWindow::
changeSerialPort_2( int )
{
    const int iface = ui->comboBox_6->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_7->currentText() );
        settings.setValue( "serialparity", ui->comboBox_8->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_9->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_10->currentText() );
        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_8->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface_2(port, parity);
        onRtuPortActive_2(true);
    }
    else emit connectionError( tr( "No serial port found at Loop_2" ) );
}


void
MainWindow::
changeSerialPort_3( int )
{
    const int iface = ui->comboBox_11->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_12->currentText() );
        settings.setValue( "serialparity", ui->comboBox_13->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_14->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_15->currentText() );
        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_13->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface_3(port, parity);
        onRtuPortActive_3(true);
    }
    else emit connectionError( tr( "No serial port found" ) );
}


void
MainWindow::
changeSerialPort_4( int )
{
    const int iface = ui->comboBox_16->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_17->currentText() );
        settings.setValue( "serialparity", ui->comboBox_18->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_19->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_20->currentText() );

        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_18->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface_4(port, parity);
        onRtuPortActive_4(true);
    }
    else emit connectionError( tr( "No serial port found" ) );
}


void
MainWindow::
changeSerialPort_5( int )
{
    const int iface = ui->comboBox_21->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_22->currentText() );
        settings.setValue( "serialparity", ui->comboBox_23->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_24->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_25->currentText() );

        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_23->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface_5(port, parity);
        onRtuPortActive_5(true);
    }
    else emit connectionError( tr( "No serial port found" ) );
}


void
MainWindow::
changeSerialPort_6( int )
{
    const int iface = ui->comboBox_26->currentIndex();

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    if( !ports.isEmpty() )
    {
        QSettings settings;
        settings.setValue( "serialinterface", ports[iface].friendName );
        settings.setValue( "serialbaudrate", ui->comboBox_27->currentText() );
        settings.setValue( "serialparity", ui->comboBox_28->currentText() );
        settings.setValue( "serialdatabits", ui->comboBox_29->currentText() );
        settings.setValue( "serialstopbits", ui->comboBox_30->currentText() );

        QString port = ports[iface].portName;

        // is it a serial port in the range COM1 .. COM9?
        if ( port.startsWith( "COM" ) )
        {
            // use windows communication device name "\\.\COMn"
            port = "\\\\.\\" + port;
        }

        char parity;
        switch( ui->comboBox_28->currentIndex() )
        {
            case 1: parity = 'O'; break;
            case 2: parity = 'E'; break;
            default:
            case 0: parity = 'N'; break;
        }

        changeModbusInterface_6(port, parity);
        onRtuPortActive_6(true);
    }
    else emit connectionError( tr( "No serial port found" ) );
}


void
MainWindow::
changeModbusInterface(const QString& port, char parity)
{
    releaseSerialModbus();

    m_serialModbus = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_2->currentText().toInt(),
            parity,
            ui->comboBox_3->currentText().toInt(),
            ui->comboBox_4->currentText().toInt() );

    if( modbus_connect( m_serialModbus ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_1!" ) );

        releaseSerialModbus();
    }
    else
        updateTabIcon(0, true);
}

void
MainWindow::
changeModbusInterface_2(const QString& port, char parity)
{
    releaseSerialModbus_2();

    m_serialModbus_2 = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_7->currentText().toInt(),
            parity,
            ui->comboBox_8->currentText().toInt(),
            ui->comboBox_9->currentText().toInt() );

    if( modbus_connect( m_serialModbus_2 ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_2!" ) );

        releaseSerialModbus_2();
    }
    else 
        updateTabIcon(1, true);
}

void
MainWindow::
changeModbusInterface_3(const QString& port, char parity)
{
    releaseSerialModbus_3();

    m_serialModbus_3 = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_12->currentText().toInt(),
            parity,
            ui->comboBox_13->currentText().toInt(),
            ui->comboBox_14->currentText().toInt() );

    if( modbus_connect( m_serialModbus_3 ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_3!" ) );

        releaseSerialModbus_3();
    }
    else
        updateTabIcon(2, true);
}

void
MainWindow::
changeModbusInterface_4(const QString& port, char parity)
{
    releaseSerialModbus_4();

    m_serialModbus_4 = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_17->currentText().toInt(),
            parity,
            ui->comboBox_18->currentText().toInt(),
            ui->comboBox_19->currentText().toInt() );

    if( modbus_connect( m_serialModbus_4 ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_4!" ) );

        releaseSerialModbus_4();
    }
    else
        updateTabIcon(3, true);
}


void
MainWindow::
changeModbusInterface_5(const QString& port, char parity)
{
    releaseSerialModbus_5();

    m_serialModbus_5 = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_22->currentText().toInt(),
            parity,
            ui->comboBox_23->currentText().toInt(),
            ui->comboBox_24->currentText().toInt() );

    if( modbus_connect( m_serialModbus_5 ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_5!" ) );

        releaseSerialModbus_5();
    }
    else
        updateTabIcon(4, true);
}

void
MainWindow::
changeModbusInterface_6(const QString& port, char parity)
{
    releaseSerialModbus_6();

    m_serialModbus_6 = modbus_new_rtu( port.toLatin1().constData(),
            ui->comboBox_27->currentText().toInt(),
            parity,
            ui->comboBox_28->currentText().toInt(),
            ui->comboBox_29->currentText().toInt() );

    if( modbus_connect( m_serialModbus_6 ) == -1 )
    {
        emit connectionError( tr( "Could not connect serial port at Loop_6" ) );

        releaseSerialModbus_6();
    }
    else
        updateTabIcon(5, true);
}

void
MainWindow::
onCheckBoxChecked(bool checked)
{
    clearMonitors();

    if (ui->tabWidget_2->currentIndex() == 0)
    {
        if (checked) setupModbusPort();
        else releaseSerialModbus();

        ui->tabWidget_3->setEnabled(checked);
        onRtuPortActive(checked);
    }
    else if (ui->tabWidget_2->currentIndex() == 1)
    {
        if (checked) setupModbusPort_2();
        else releaseSerialModbus_2();

        ui->tabWidget_4->setEnabled(checked);
        onRtuPortActive_2(checked);
    }   
    else if (ui->tabWidget_2->currentIndex() == 2)
    {
        if (checked) setupModbusPort_3();
        else releaseSerialModbus_3();

        ui->tabWidget_5->setEnabled(checked);
        onRtuPortActive_3(checked);
    }   
    else if (ui->tabWidget_2->currentIndex() == 3)
    {
        if (checked) setupModbusPort_4();
        else releaseSerialModbus_4();

        ui->tabWidget_6->setEnabled(checked);
        onRtuPortActive_4(checked);
    }   
    else if (ui->tabWidget_2->currentIndex() == 4)
    {
        if (checked) setupModbusPort_5();
        else releaseSerialModbus_5();

        ui->tabWidget_7->setEnabled(checked);
        onRtuPortActive_5(checked);
    }
    else
    {
        if (checked) setupModbusPort_6();
        else releaseSerialModbus_6();

        ui->tabWidget_8->setEnabled(checked);
        onRtuPortActive_6(checked);
    }   
}


void
MainWindow::
clearMonitors()
{
    ui->rawData->clear();
    ui->regTable->setRowCount(0);
    ui->busMonTable->setRowCount(0);
}

/*
void
MainWindow::
connectRegisters()
{
    connect(ui->radioButton_2, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_10, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_16, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_28, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_34, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_38, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_46, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_70, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_74, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_82, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_92, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_100, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
    connect(ui->radioButton_106, SIGNAL(pressed()), this,  SLOT(onProductBtnPressed()));
}

void
MainWindow::
onProductBtnPressed()
{
    (ui->radioButton_2->isChecked()) ? updateRegisters(RAZ,0) : updateRegisters(EEA,0);
    (ui->radioButton_10->isChecked()) ? updateRegisters(RAZ,1) : updateRegisters(EEA,1);
    (ui->radioButton_16->isChecked()) ? updateRegisters(RAZ,2) : updateRegisters(EEA,2);
    (ui->radioButton_20->isChecked()) ? updateRegisters(RAZ,3) : updateRegisters(EEA,3);
    (ui->radioButton_28->isChecked()) ? updateRegisters(RAZ,4) : updateRegisters(EEA,4);
    (ui->radioButton_34->isChecked()) ? updateRegisters(RAZ,5) : updateRegisters(EEA,5);
    (ui->radioButton_38->isChecked()) ? updateRegisters(RAZ,6) : updateRegisters(EEA,6);
    (ui->radioButton_46->isChecked()) ? updateRegisters(RAZ,7) : updateRegisters(EEA,7);
    (ui->radioButton_52->isChecked()) ? updateRegisters(RAZ,8) : updateRegisters(EEA,8);
    (ui->radioButton_56->isChecked()) ? updateRegisters(RAZ,9) : updateRegisters(EEA,8);
    (ui->radioButton_64->isChecked()) ? updateRegisters(RAZ,10) : updateRegisters(EEA,10);
    (ui->radioButton_70->isChecked()) ? updateRegisters(RAZ,11) : updateRegisters(EEA,11);
    (ui->radioButton_74->isChecked()) ? updateRegisters(RAZ,12) : updateRegisters(EEA,12);
    (ui->radioButton_82->isChecked()) ? updateRegisters(RAZ,13) : updateRegisters(EEA,13);
    (ui->radioButton_88->isChecked()) ? updateRegisters(RAZ,14) : updateRegisters(EEA,14);
    (ui->radioButton_92->isChecked()) ? updateRegisters(RAZ,15) : updateRegisters(EEA,15);
    (ui->radioButton_100->isChecked()) ? updateRegisters(RAZ,16) : updateRegisters(EEA,16);
    (ui->radioButton_106->isChecked()) ? updateRegisters(RAZ,17) : updateRegisters(EEA,17);
}
*/

void
MainWindow::
onLoopTabChanged(int index)
{
    clearMonitors();    

    if (index == 0)
    {
        if (ui->tabWidget_3->currentIndex() == 0)
        {
            (ui->radioButton_2->isChecked()) ? updateRegisters(RAZ,0) : updateRegisters(EEA,0);
        }
        else if (ui->tabWidget_3->currentIndex() == 1)
        {
            (ui->radioButton_22->isChecked()) ? updateRegisters(RAZ,1) : updateRegisters(EEA,1);
        }
        else
        {
            (ui->radioButton_70->isChecked()) ? updateRegisters(RAZ,2) : updateRegisters(EEA,2);
        }
    } 
    else if (index == 1)
    {
        if (ui->tabWidget_4->currentIndex() == 0)
        {
            (ui->radioButton_82->isChecked()) ? updateRegisters(RAZ,3) : updateRegisters(EEA,3);
        }
        else if (ui->tabWidget_4->currentIndex() == 1)
        {
            (ui->radioButton_94->isChecked()) ? updateRegisters(RAZ,4) : updateRegisters(EEA,4);
        }
        else
        {
            (ui->radioButton_106->isChecked()) ? updateRegisters(RAZ,5) : updateRegisters(EEA,5);
        }
    }
    else if (index == 2)
    {
        if (ui->tabWidget_5->currentIndex() == 0)
        {
            (ui->radioButton_118->isChecked()) ? updateRegisters(RAZ,6) : updateRegisters(EEA,6);
        }
        else if (ui->tabWidget_5->currentIndex() == 1)
        {
            (ui->radioButton_130->isChecked()) ? updateRegisters(RAZ,7) : updateRegisters(EEA,7);
        }
        else
        {
            (ui->radioButton_142->isChecked()) ? updateRegisters(RAZ,8) : updateRegisters(EEA,8);
        }
    }
    else if (index == 3)
    {

        if (ui->tabWidget_6->currentIndex() == 0)
        {
            (ui->radioButton_154->isChecked()) ? updateRegisters(RAZ,9) : updateRegisters(EEA,9);
        }
        else if (ui->tabWidget_6->currentIndex() == 1)
        {
            (ui->radioButton_166->isChecked()) ? updateRegisters(RAZ,10) : updateRegisters(EEA,10);
        }
        else
        {
            (ui->radioButton_178->isChecked()) ? updateRegisters(RAZ,11) : updateRegisters(EEA,11);
        }
    }
    else if (index == 4)
    {

        if (ui->tabWidget_7->currentIndex() == 0)
        {
            (ui->radioButton_203->isChecked()) ? updateRegisters(RAZ,12) : updateRegisters(EEA,12);
        }
        else if (ui->tabWidget_7->currentIndex() == 1)
        {
            (ui->radioButton_215->isChecked()) ? updateRegisters(RAZ,13) : updateRegisters(EEA,13);
        }
        else
        {
            (ui->radioButton_227->isChecked()) ? updateRegisters(RAZ,14) : updateRegisters(EEA,14);
        }
    }
    else
    {

        if (ui->tabWidget_8->currentIndex() == 0)
        {
            (ui->radioButton_239->isChecked()) ? updateRegisters(RAZ,15) : updateRegisters(EEA,15);
        }
        else if (ui->tabWidget_8->currentIndex() == 1)
        {
            (ui->radioButton_251->isChecked()) ? updateRegisters(RAZ,16) : updateRegisters(EEA,16);
        }
        else
        {
            (ui->radioButton_263->isChecked()) ? updateRegisters(RAZ,17) : updateRegisters(EEA,17);
        }
    }

    updateGraph();
}


void
MainWindow::
updateTabIcon(int index, bool connected)
{
    QIcon icon(QLatin1String(":/green.ico"));
    QIcon icoff(QLatin1String(":/red.ico"));
    (connected) ? ui->tabWidget_2->setTabIcon(index,icon) : ui->tabWidget_2->setTabIcon(index,icoff);
}


void
MainWindow::
initializeTabIcons()
{
    QIcon icon(QLatin1String(":/red.ico"));
    for (int i = 0; i < 6; i++) ui->tabWidget_2->setTabIcon(i,icon);
}


void
MainWindow::
updateChartTitle()
{
         if ((ui->tabWidget_2->currentIndex() == 0) && (ui->tabWidget_3->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 1");
    else if ((ui->tabWidget_2->currentIndex() == 0) && (ui->tabWidget_3->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 1");
    else if ((ui->tabWidget_2->currentIndex() == 0) && (ui->tabWidget_3->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 1");
    else if ((ui->tabWidget_2->currentIndex() == 1) && (ui->tabWidget_4->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 2");
    else if ((ui->tabWidget_2->currentIndex() == 1) && (ui->tabWidget_4->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 2");
    else if ((ui->tabWidget_2->currentIndex() == 1) && (ui->tabWidget_4->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 2");
    else if ((ui->tabWidget_2->currentIndex() == 2) && (ui->tabWidget_5->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 3");
    else if ((ui->tabWidget_2->currentIndex() == 2) && (ui->tabWidget_5->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 3");
    else if ((ui->tabWidget_2->currentIndex() == 2) && (ui->tabWidget_5->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 3");
    else if ((ui->tabWidget_2->currentIndex() == 3) && (ui->tabWidget_6->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 4");
    else if ((ui->tabWidget_2->currentIndex() == 3) && (ui->tabWidget_6->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 4");
    else if ((ui->tabWidget_2->currentIndex() == 3) && (ui->tabWidget_6->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 4");
    else if ((ui->tabWidget_2->currentIndex() == 4) && (ui->tabWidget_7->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 5");
    else if ((ui->tabWidget_2->currentIndex() == 4) && (ui->tabWidget_7->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 5");
    else if ((ui->tabWidget_2->currentIndex() == 4) && (ui->tabWidget_7->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 5");
    else if ((ui->tabWidget_2->currentIndex() == 5) && (ui->tabWidget_8->currentIndex() == 0)) chart->setTitle("Pipe 1 at Loop 6");
    else if ((ui->tabWidget_2->currentIndex() == 5) && (ui->tabWidget_8->currentIndex() == 1)) chart->setTitle("Pipe 2 at Loop 6");
    else if ((ui->tabWidget_2->currentIndex() == 5) && (ui->tabWidget_8->currentIndex() == 2)) chart->setTitle("Pipe 3 at Loop 6");
}


void
MainWindow::
connectCalibrationControls()
{
    connect(ui->pushButton_4, SIGNAL(pressed()), this, SLOT(calibration_L1P1()));
    connect(ui->pushButton_5, SIGNAL(pressed()), this, SLOT(calibration_L1P2()));
    connect(ui->pushButton_8, SIGNAL(pressed()), this, SLOT(calibration_L1P3()));
    connect(ui->pushButton_9, SIGNAL(pressed()), this, SLOT(calibration_L2P1()));
    connect(ui->pushButton_10, SIGNAL(pressed()), this, SLOT(calibration_L2P2()));
    connect(ui->pushButton_11, SIGNAL(pressed()), this, SLOT(calibration_L2P3()));
    connect(ui->pushButton_12, SIGNAL(pressed()), this, SLOT(calibration_L3P1()));
    connect(ui->pushButton_13, SIGNAL(pressed()), this, SLOT(calibration_L3P2()));
    connect(ui->pushButton_14, SIGNAL(pressed()), this, SLOT(calibration_L3P3()));
    connect(ui->pushButton_15, SIGNAL(pressed()), this, SLOT(calibration_L4P1()));
    connect(ui->pushButton_16, SIGNAL(pressed()), this, SLOT(calibration_L4P2()));
    connect(ui->pushButton_17, SIGNAL(pressed()), this, SLOT(calibration_L4P3()));
    connect(ui->pushButton_18, SIGNAL(pressed()), this, SLOT(calibration_L5P1()));
    connect(ui->pushButton_19, SIGNAL(pressed()), this, SLOT(calibration_L5P2()));
    connect(ui->pushButton_20, SIGNAL(pressed()), this, SLOT(calibration_L5P3()));
    connect(ui->pushButton_21, SIGNAL(pressed()), this, SLOT(calibration_L6P1()));
    connect(ui->pushButton_22, SIGNAL(pressed()), this, SLOT(calibration_L6P2()));
    connect(ui->pushButton_23, SIGNAL(pressed()), this, SLOT(calibration_L6P3()));
}


void
MainWindow::
createLoopFiles(const int sn, const QString path, const BOOL iseea, const QString max_salt, const QString min_salt, const QString oil_temp, const QString v_olume, const QString max_water_run, const QString min_water_run, const QString max_oil_run, const QString min_oil_run, QFile & file1, QFile & file2, QFile & file3, QFile & file4, QFile & file5, QFile & file6, QFile & file7)
{
    QDateTime currentDataTime = QDateTime::currentDateTime();

    QDir dir;
    QString filePath;
    QString cutMode;
    int fileCounter = 1;

    if (path == HIGH) cutMode = "HIGHCUT";
    else if (path == FULL) cutMode = "FULLCUT";
    else if (path == MID) cutMode = "MIDCUT";
    else if (path == LOW) cutMode = "LOWCUT";
    
    QString startWaterRun = max_water_run;
    QString stopWaterRun = min_water_run;
    QString startOilRun = max_oil_run;
    QString stopOilRun = min_oil_run;
    QString startSalt = min_salt;
    QString stopSalt = max_salt;
    QString oilTemp = oil_temp;
    QString volume = v_olume;

    QString header0 = "EEA INJECTION FILE";
    QString header1("SN"+QString::number(sn)+" | "+cutMode +" | "+currentDataTime.toString()+" | "+PROJECT+RELEASE_VERSION); // SN1894 | HIGHCUT | Mon Dec 14 10:21:20 2020 | Sparky 0.0.7
    QString header2("INJECTION:  "+startWaterRun+" % "+"to "+stopWaterRun+" % "+"Watercut at "+startSalt+" % "+"Salinity\n");
    QString header3("Time From  Water  Osc  Tune Tuning            Incident Reflected                         Analog     User Input  Injection");
    QString header4("Run Start   Cut   Band Type Voltage Frequency  Power     Power   Temperature Pressure    Input        Value       Time     Comment");
    QString header5("========= ======= ==== ==== ======= ========= ======== ========= =========== ======== ============ ============ ========== ============");

    QTextStream stream1(&file1);
    QTextStream stream2(&file2);
    QTextStream stream3(&file3);
    QTextStream stream4(&file4);
    QTextStream stream5(&file5);
    QTextStream stream6(&file6);
    QTextStream stream7(&file7);

    /// We create the directory if needed
    if (!dir.exists(path+QString::number(sn))) 
    {
        filePath = path+QString::number(sn);
        dir.mkpath(filePath);
    } 
    else
    {
        while (1)
        {
            if (!dir.exists(path+QString::number(sn)+"_"+QString::number(fileCounter))) 
            {
                filePath = path+QString::number(sn)+"_"+QString::number(fileCounter);
                dir.mkpath(filePath);
                break;
            }
            else fileCounter++;
        }
    }

    /// set filenames
    if (path == HIGH) 
    {
        file1.setFileName(filePath+"\\"+FILE_LIST_HC);
        file2.setFileName(filePath+"\\"+AMB_TWENTY_HC);
        file3.setFileName(filePath+"\\"+TWENTY_FIFTYFIVE_HC);
        file4.setFileName(filePath+"\\"+FIFTYFIVE_THIRTYEIGHT_HC);
        file5.setFileName(filePath+"\\"+CALIBRAT_HC);
        file6.setFileName(filePath+"\\"+ADJUSTED_HC);
        file7.setFileName(filePath+"\\"+ROLLOVER_HC);
    }
    else if (path == FULL) 
    {
        file1.setFileName(filePath+"\\"+FILE_LIST_FC);
        file2.setFileName(filePath+"\\"+AMB_TWENTY_FC);
        file3.setFileName(filePath+"\\"+TWENTY_FIFTYFIVE_FC);
        file4.setFileName(filePath+"\\"+FIFTYFIVE_THIRTYEIGHT_FC);
        file5.setFileName(filePath+"\\"+CALIBRAT_FC);
        file6.setFileName(filePath+"\\"+ADJUSTED_FC);
        file7.setFileName(filePath+"\\"+ROLLOVER_FC);
    }
    else if (path == MID) 
    {
        file1.setFileName(filePath+"\\"+FILE_LIST_MC);
        file2.setFileName(filePath+"\\"+AMB_TWENTY_MC);
        file3.setFileName(filePath+"\\"+TWENTY_FIFTYFIVE_MC);
        file4.setFileName(filePath+"\\"+FIFTYFIVE_THIRTYEIGHT_MC);
        file5.setFileName(filePath+"\\"+CALIBRAT_MC);
        file6.setFileName(filePath+"\\"+ADJUSTED_MC);
        file7.setFileName(filePath+"\\"+ROLLOVER_MC);
    }
    else if (path == LOW) 
    {
        file1.setFileName(filePath+"\\"+FILE_LIST_LC);
        file2.setFileName(filePath+"\\"+AMB_TWENTY_LC);
        file3.setFileName(filePath+"\\"+TWENTY_FIFTYFIVE_LC);
        file4.setFileName(filePath+"\\"+FIFTYFIVE_THIRTYEIGHT_LC);
        file5.setFileName(filePath+"\\"+CALIBRAT_LC);
        file6.setFileName(filePath+"\\"+ADJUSTED_LC);
        file7.setFileName(filePath+"\\"+ROLLOVER_LC);
    }
    else return; // never reaches here

    /// product
    if (iseea) header0 = EEA_INJECTION_FILE;
    else header0 = RAZ_INJECTION_FILE;

    /// open files
    file1.open(QIODevice::WriteOnly | QIODevice::Text);
    file2.open(QIODevice::WriteOnly | QIODevice::Text);
    file3.open(QIODevice::WriteOnly | QIODevice::Text);
    file4.open(QIODevice::WriteOnly | QIODevice::Text);
    file5.open(QIODevice::WriteOnly | QIODevice::Text);
    file6.open(QIODevice::WriteOnly | QIODevice::Text);
    file7.open(QIODevice::WriteOnly | QIODevice::Text);

    /// write headers
    stream1 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream2 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream3 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream4 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream5 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream6 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
    stream7 << header0 << '\n' << header1 << '\n' << header2 << '\n' << header3 << '\n' << header4 << '\n' << header5 << '\n';
                    
    /// close files
    file1.close();
    file2.close();
    file3.close();
    file4.close();
    file5.close();
    file6.close();
    file7.close();
}


void
MainWindow::
calibration_L1P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_2->text().toInt();
    QString startSalt = ui->comboBox_31->currentText();
    QString stopSalt = ui->comboBox_33->currentText();
    QString oilTemp = ui->comboBox_32->currentText();
    QString volume = ui->lineEdit->text();
    QString startWaterRun = ui->lineEdit_37->text();
    QString stopWaterRun = ui->lineEdit_38->text();
    QString startOilRun = ui->lineEdit_39->text();
    QString stopOilRun = ui->lineEdit_40->text();

    if (m_modbus == NULL) // LOOP 1
    {
        setStatusError( tr("Loop_1 not configured!") );
        return;       
    }

    if (ui->lineEdit_2->text().isEmpty())
    {
        setStatusError( tr("Loop_1_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_3->isChecked()) path = HIGH;          // HIGH
    else if (ui->radioButton_4->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_5->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_6->isChecked()) path = LOW;      // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L1P1, file2_L1P1, file3_L1P1, file4_L1P1, file5_L1P1, file6_L1P1, file7_L1P1);

    /// control group box
    ui->pushButton_4->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L1P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_7->text().toInt();
    QString startSalt = ui->comboBox_42->currentText();
    QString stopSalt = ui->comboBox_41->currentText();
    QString oilTemp = ui->comboBox_40->currentText();
    QString volume = ui->lineEdit_8->text();
    QString startWaterRun = ui->lineEdit_51->text();
    QString stopWaterRun = ui->lineEdit_52->text();
    QString startOilRun = ui->lineEdit_49->text();
    QString stopOilRun = ui->lineEdit_50->text();

    if (m_modbus == NULL)
    {
        setStatusError( tr("Loop_1 not configured!") );
        return;       
    }

    if (ui->lineEdit_7->text().isEmpty())
    {
        setStatusError( tr("Loop_1_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_13->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_23->isChecked()) path = HIGH;          // HIGH
    else if (ui->radioButton_24->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_37->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_38->isChecked()) path = LOW;      // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L1P2, file2_L1P2, file3_L1P2, file4_L1P2, file5_L1P2, file6_L1P2, file7_L1P2);

    ui->pushButton_5->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L1P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_13->text().toInt();
    QString startSalt = ui->comboBox_51->currentText();
    QString stopSalt = ui->comboBox_50->currentText();
    QString oilTemp = ui->comboBox_49->currentText();
    QString volume = ui->lineEdit_14->text();
    QString startWaterRun = ui->lineEdit_63->text();
    QString stopWaterRun = ui->lineEdit_64->text();
    QString startOilRun = ui->lineEdit_61->text();
    QString stopOilRun = ui->lineEdit_62->text();

    if (m_modbus == NULL)
    {
        setStatusError( tr("Loop_1 not configured!") );
        return;       
    }

    if (ui->lineEdit_13->text().isEmpty())
    {
        setStatusError( tr("Loop_1_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_69->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_71->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_72->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_73->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_74->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L1P3, file2_L1P3, file3_L1P3, file4_L1P3, file5_L1P3, file6_L1P3, file7_L1P3);

    ui->pushButton_8->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L2P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_15->text().toInt();
    QString startSalt = ui->comboBox_53->currentText();
    QString stopSalt = ui->comboBox_52->currentText();
    QString oilTemp = ui->comboBox_54->currentText();
    QString volume = ui->lineEdit_16->text();
    QString startWaterRun = ui->lineEdit_67->text();
    QString stopWaterRun = ui->lineEdit_68->text();
    QString startOilRun = ui->lineEdit_65->text();
    QString stopOilRun = ui->lineEdit_66->text();

    if (m_modbus_2 == NULL)
    {
        setStatusError( tr("Loop_2 not configured!") );
        return;       
    }

    if (ui->lineEdit_15->text().isEmpty())
    {
        setStatusError( tr("Loop_2_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_81->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_83->isChecked()) path = HIGH;          // HIGH
    else if (ui->radioButton_84->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_85->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_86->isChecked()) path = LOW;      // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L2P1, file2_L2P1, file3_L2P1, file4_L2P1, file5_L2P1, file6_L2P1, file7_L2P1);

    ui->pushButton_9->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L2P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_17->text().toInt();
    QString startSalt = ui->comboBox_57->currentText();
    QString stopSalt = ui->comboBox_56->currentText();
    QString oilTemp = ui->comboBox_55->currentText();
    QString volume = ui->lineEdit_18->text();
    QString startWaterRun = ui->lineEdit_71->text();
    QString stopWaterRun = ui->lineEdit_72->text();
    QString startOilRun = ui->lineEdit_69->text();
    QString stopOilRun = ui->lineEdit_70->text();

    if (m_modbus_2 == NULL) // LOOP2 
    {
        setStatusError( tr("Loop_2 not configured!") );
        return;       
    }

    if (ui->lineEdit_17->text().isEmpty())
    {
        setStatusError( tr("Loop_2_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_93->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_95->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_96->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_97->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_98->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L2P2, file2_L2P2, file3_L2P2, file4_L2P2, file5_L2P2, file6_L2P2, file7_L2P2);

    ui->pushButton_10->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L2P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_19->text().toInt();
    QString startSalt = ui->comboBox_60->currentText();
    QString stopSalt = ui->comboBox_59->currentText();
    QString oilTemp = ui->comboBox_58->currentText();
    QString volume = ui->lineEdit_20->text();
    QString startWaterRun = ui->lineEdit_75->text();
    QString stopWaterRun = ui->lineEdit_76->text();
    QString startOilRun = ui->lineEdit_73->text();
    QString stopOilRun = ui->lineEdit_74->text();

    if (m_modbus_2 == NULL)
    {
        setStatusError( tr("Loop_2 not configured!") );
        return;       
    }

    if (ui->lineEdit_19->text().isEmpty())
    {
        setStatusError( tr("Loop_2_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_105->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_107->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_108->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_109->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_110->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L2P3, file2_L2P3, file3_L2P3, file4_L2P3, file5_L2P3, file6_L2P3, file7_L2P3);

    ui->pushButton_11->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L3P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_21->text().toInt();
    QString startSalt = ui->comboBox_62->currentText();
    QString stopSalt = ui->comboBox_61->currentText();
    QString oilTemp = ui->comboBox_63->currentText();
    QString volume = ui->lineEdit_22->text();
    QString startWaterRun = ui->lineEdit_79->text();
    QString stopWaterRun = ui->lineEdit_80->text();
    QString startOilRun = ui->lineEdit_77->text();
    QString stopOilRun = ui->lineEdit_78->text();

    if (m_modbus_3 == NULL) // LOOP 3
    {
        setStatusError( tr("Loop_3 not configured!") );
        return;       
    }

    if (ui->lineEdit_21->text().isEmpty())
    {
        setStatusError( tr("Loop_3_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_117->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_119->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_120->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_121->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_122->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L3P1, file2_L3P1, file3_L3P1, file4_L3P1, file5_L3P1, file6_L3P1, file7_L3P1);

    ui->pushButton_12->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L3P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_23->text().toInt();
    QString startSalt = ui->comboBox_66->currentText();
    QString stopSalt = ui->comboBox_65->currentText();
    QString oilTemp = ui->comboBox_64->currentText();
    QString volume = ui->lineEdit_24->text();
    QString startWaterRun = ui->lineEdit_83->text();
    QString stopWaterRun = ui->lineEdit_84->text();
    QString startOilRun = ui->lineEdit_81->text();
    QString stopOilRun = ui->lineEdit_82->text();

    if (m_modbus_3 == NULL)
    {
        setStatusError( tr("Loop_3 not configured!") );
        return;       
    }

    if (ui->lineEdit_23->text().isEmpty())
    {
        setStatusError( tr("Loop_3_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_129->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_131->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_132->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_133->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_134->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L3P2, file2_L3P2, file3_L3P2, file4_L3P2, file5_L3P2, file6_L3P2, file7_L3P2);

    ui->pushButton_13->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L3P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_25->text().toInt();
    QString startSalt = ui->comboBox_69->currentText();
    QString stopSalt = ui->comboBox_68->currentText();
    QString oilTemp = ui->comboBox_67->currentText();
    QString volume = ui->lineEdit_26->text();
    QString startWaterRun = ui->lineEdit_87->text();
    QString stopWaterRun = ui->lineEdit_88->text();
    QString startOilRun = ui->lineEdit_85->text();
    QString stopOilRun = ui->lineEdit_86->text();

    if (m_modbus_3 == NULL)
    {
        setStatusError( tr("Loop_1 not configured!") );
        return;       
    }

    if (ui->lineEdit_25->text().isEmpty())
    {
        setStatusError( tr("Loop_3_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_141->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_143->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_144->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_145->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_146->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L3P3, file2_L3P3, file3_L3P3, file4_L3P3, file5_L3P3, file6_L3P3, file7_L3P3);

    ui->pushButton_14->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L4P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_27->text().toInt();
    QString startSalt = ui->comboBox_71->currentText();
    QString stopSalt = ui->comboBox_70->currentText();
    QString oilTemp = ui->comboBox_72->currentText();
    QString volume = ui->lineEdit_28->text();
    QString startWaterRun = ui->lineEdit_91->text();
    QString stopWaterRun = ui->lineEdit_92->text();
    QString startOilRun = ui->lineEdit_89->text();
    QString stopOilRun = ui->lineEdit_90->text();

    if (m_modbus_4 == NULL)
    {
        setStatusError( tr("Loop_4 not configured!") );
        return;       
    }

    if (ui->lineEdit_27->text().isEmpty())
    {
        setStatusError( tr("Loop_4_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_153->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_155->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_156->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_157->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_158->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L4P1, file2_L4P1, file3_L4P1, file4_L4P1, file5_L4P1, file6_L4P1, file7_L4P1);

    ui->pushButton_15->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L4P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_29->text().toInt();
    QString startSalt = ui->comboBox_75->currentText();
    QString stopSalt = ui->comboBox_74->currentText();
    QString oilTemp = ui->comboBox_73->currentText();
    QString volume = ui->lineEdit_30->text();
    QString startWaterRun = ui->lineEdit_95->text();
    QString stopWaterRun = ui->lineEdit_96->text();
    QString startOilRun = ui->lineEdit_93->text();
    QString stopOilRun = ui->lineEdit_94->text();

    if (m_modbus_4 == NULL) // LOOP4 
    {
        setStatusError( tr("Loop_4 not configured!") );
        return;       
    }

    if (ui->lineEdit_29->text().isEmpty())
    {
        setStatusError( tr("Loop_4_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_165->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_167->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_168->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_169->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_170->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L4P2, file2_L4P2, file3_L4P2, file4_L4P2, file5_L4P2, file6_L4P2, file7_L4P2);

    ui->pushButton_16->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L4P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_31->text().toInt();
    QString startSalt = ui->comboBox_78->currentText();
    QString stopSalt = ui->comboBox_77->currentText();
    QString oilTemp = ui->comboBox_76->currentText();
    QString volume = ui->lineEdit_32->text();
    QString startWaterRun = ui->lineEdit_99->text();
    QString stopWaterRun = ui->lineEdit_100->text();
    QString startOilRun = ui->lineEdit_97->text();
    QString stopOilRun = ui->lineEdit_98->text();

    if (m_modbus_4 == NULL)
    {
        setStatusError( tr("Loop_4 not configured!") );
        return;       
    }

    if (ui->lineEdit_2->text().isEmpty())
    {
        setStatusError( tr("Loop_4_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_177->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_179->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_180->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_194->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_195->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L4P3, file2_L4P3, file3_L4P3, file4_L4P3, file5_L4P3, file6_L4P3, file7_L4P3);

    ui->pushButton_17->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L5P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_33->text().toInt();
    QString startSalt = ui->comboBox_80->currentText();
    QString stopSalt = ui->comboBox_79->currentText();
    QString oilTemp = ui->comboBox_81->currentText();
    QString volume = ui->lineEdit_34->text();
    QString startWaterRun = ui->lineEdit_103->text();
    QString stopWaterRun = ui->lineEdit_104->text();
    QString startOilRun = ui->lineEdit_101->text();
    QString stopOilRun = ui->lineEdit_102->text();

    if (m_modbus_5 == NULL) // LOOP 1
    {
        setStatusError( tr("Loop_5 not configured!") );
        return;       
    }

    if (ui->lineEdit_33->text().isEmpty())
    {
        setStatusError( tr("Loop_5_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_202->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_204->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_205->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_206->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_207->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L5P1, file2_L5P1, file3_L5P1, file4_L5P1, file5_L5P1, file6_L5P1, file7_L5P1);

    ui->pushButton_18->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L5P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_35->text().toInt();
    QString startSalt = ui->comboBox_84->currentText();
    QString stopSalt = ui->comboBox_83->currentText();
    QString oilTemp = ui->comboBox_82->currentText();
    QString volume = ui->lineEdit_36->text();
    QString startWaterRun = ui->lineEdit_107->text();
    QString stopWaterRun = ui->lineEdit_108->text();
    QString startOilRun = ui->lineEdit_105->text();
    QString stopOilRun = ui->lineEdit_106->text();

    if (m_modbus_5 == NULL)
    {
        setStatusError( tr("Loop_5 not configured!") );
        return;       
    }

    if (ui->lineEdit_35->text().isEmpty())
    {
        setStatusError( tr("Loop_5_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_214->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_216->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_217->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_218->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_219->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L5P2, file2_L5P2, file3_L5P2, file4_L5P2, file5_L5P2, file6_L5P2, file7_L5P2);

    ui->pushButton_19->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L5P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_110->text().toInt();
    QString startSalt = ui->comboBox_87->currentText();
    QString stopSalt = ui->comboBox_86->currentText();
    QString oilTemp = ui->comboBox_85->currentText();
    QString volume = ui->lineEdit_114->text();
    QString startWaterRun = ui->lineEdit_115->text();
    QString stopWaterRun = ui->lineEdit_116->text();
    QString startOilRun = ui->lineEdit_112->text();
    QString stopOilRun = ui->lineEdit_113->text();

    if (m_modbus_5 == NULL) // LOOP 1
    {
        setStatusError( tr("Loop_5 not configured!") );
        return;       
    }

    if (ui->lineEdit_110->text().isEmpty())
    {
        setStatusError( tr("Loop_5_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_226->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_228->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_229->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_230->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_231->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L5P3, file2_L5P3, file3_L5P3, file4_L5P3, file5_L5P3, file6_L5P3, file7_L5P3);

    ui->pushButton_20->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L6P1()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_121->text().toInt();
    QString startSalt = ui->comboBox_89->currentText();
    QString stopSalt = ui->comboBox_88->currentText();
    QString oilTemp = ui->comboBox_90->currentText();
    QString volume = ui->lineEdit_122->text();
    QString startWaterRun = ui->lineEdit_119->text();
    QString stopWaterRun = ui->lineEdit_120->text();
    QString startOilRun = ui->lineEdit_117->text();
    QString stopOilRun = ui->lineEdit_118->text();

    if (m_modbus_6 == NULL)
    {
        setStatusError( tr("Loop_6 not configured!") );
        return;       
    }

    if (ui->lineEdit_121->text().isEmpty())
    {
        setStatusError( tr("Loop_6_Pipe_1 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_238->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_240->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_241->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_242->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_243->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L6P1, file2_L6P1, file3_L6P1, file4_L6P1, file5_L6P1, file6_L6P1, file7_L6P1);

    ui->pushButton_21->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L6P2()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_123->text().toInt();
    QString startSalt = ui->comboBox_93->currentText();
    QString stopSalt = ui->comboBox_92->currentText();
    QString oilTemp = ui->comboBox_91->currentText();
    QString volume = ui->lineEdit_126->text();
    QString startWaterRun = ui->lineEdit_127->text();
    QString stopWaterRun = ui->lineEdit_128->text();
    QString startOilRun = ui->lineEdit_124->text();
    QString stopOilRun = ui->lineEdit_125->text();

    if (m_modbus_6 == NULL)
    {
        setStatusError( tr("Loop_6 not configured!") );
        return;       
    }

    if (ui->lineEdit_123->text().isEmpty())
    {
        setStatusError( tr("Loop_6_Pipe_2 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_250->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_252->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_253->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_254->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_255->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L6P2, file2_L6P2, file3_L6P2, file4_L6P2, file5_L6P2, file6_L6P2, file7_L6P2);

    ui->pushButton_22->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
calibration_L6P3()
{
    QString path;
    BOOL isEEA = true;

    /// get user inputs
    int slave = ui->lineEdit_129->text().toInt();
    QString startSalt = ui->comboBox_96->currentText();
    QString stopSalt = ui->comboBox_95->currentText();
    QString oilTemp = ui->comboBox_94->currentText();
    QString volume = ui->lineEdit_132->text();
    QString startWaterRun = ui->lineEdit_133->text();
    QString stopWaterRun = ui->lineEdit_134->text();
    QString startOilRun = ui->lineEdit_130->text();
    QString stopOilRun = ui->lineEdit_131->text();

    if (m_modbus_6 == NULL)
    {
        setStatusError( tr("Loop_6 not configured!") );
        return;       
    }

    if (ui->lineEdit_129->text().isEmpty())
    {
        setStatusError( tr("Loop_6_Pipe_3 no serial number!") );
        return;       
    }

/*    const int addr = ui->startAddr->value()-1;
    uint8_t dest[1024];
    uint16_t * dest16 = (uint16_t *) dest;
    int ret = -1;
    bool is16Bit = false;
    bool writeAccess = false;
    const QString funcType = descriptiveDataTypeName( FUNC_READ_FLOAT );
*/
   
    /// product
    if (ui->radioButton_262->isChecked()) isEEA = true;
    else isEEA = false;

    /// cut
    if (ui->radioButton_264->isChecked()) path = HIGH;            // HIGH
    else if (ui->radioButton_265->isChecked()) path = FULL;     // FULL
    else if (ui->radioButton_266->isChecked()) path = MID ;     // MID
    else if (ui->radioButton_267->isChecked()) path = LOW;        // LOW

    /// create files
    createLoopFiles(slave, path, isEEA, startSalt, stopSalt, oilTemp, volume, startWaterRun, stopWaterRun, startOilRun, stopOilRun, file1_L6P3, file2_L6P3, file3_L6P3, file4_L6P3, file5_L6P3, file6_L6P3, file7_L6P3);

    ui->pushButton_23->setText(tr("S T O P"));
/*    memset( dest, 0, 1024 );
    modbus_set_slave( m_serialModbus, slave );
    sendCalibrationRequest(FLOAT_R, m_serialModbus, FUNC_READ_FLOAT, addr, BYTE_READ_FLOAT, ret, dest, dest16, is16Bit, writeAccess, funcType);
*/
}


void
MainWindow::
onFunctionCodeChanges()
{
    if (ui->radioButton_181->isChecked()) // float
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(3);
        }
        else
        {
            ui->functionCode->setCurrentIndex(7);
        }
    }
    else if (ui->radioButton_182->isChecked()) // integer
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(3);
        }
        else
        {
            ui->functionCode->setCurrentIndex(5);
        }
    }
    else // coil
    {
        if (ui->radioButton_187->isChecked()) // read
        {
            ui->functionCode->setCurrentIndex(0);
        }
        else
        {
            ui->functionCode->setCurrentIndex(4);
        }
    }
}


void
MainWindow::
onFloatButtonPressed(bool enabled)
{
    if (enabled) ui->numCoils->setValue(2); // quantity
    if (ui->radioButton_187->isChecked())
    {
        onReadButtonPressed(true);
    }
    else
    {
        onWriteButtonPressed(true);
    }

    ui->groupBox_103->setEnabled(TRUE);
    ui->groupBox_106->setEnabled(FALSE);
    ui->groupBox_107->setEnabled(FALSE);
    ui->lineEdit_111->clear();
}

void
MainWindow::
onIntegerButtonPressed(bool enabled)
{
    if (enabled) ui->numCoils->setValue(1);
    if (ui->radioButton_187->isChecked())
    {
        onReadButtonPressed(true);
    }
    else
    {
        onWriteButtonPressed(true);
    }
    ui->groupBox_103->setEnabled(FALSE);
    ui->groupBox_106->setEnabled(TRUE);
    ui->groupBox_107->setEnabled(FALSE);
    ui->lineEdit_109->clear();
 }

void
MainWindow::
onCoilButtonPressed(bool enabled)
{
    if (enabled) ui->numCoils->setValue(1);
    if (ui->radioButton_187->isChecked())
    {
        onReadButtonPressed(true);
    }
    else
    {
        onWriteButtonPressed(true);
    }

    ui->groupBox_103->setEnabled(FALSE);
    ui->groupBox_106->setEnabled(FALSE);
    ui->groupBox_107->setEnabled(TRUE);
    ui->lineEdit_109->clear();
    ui->lineEdit_111->clear();
}

void
MainWindow::
onReadButtonPressed(bool enabled)
{
    if (enabled)
    {
        ui->lineEdit_109->setReadOnly(true);
        ui->lineEdit_111->setReadOnly(true);
        ui->groupBox_107->setEnabled(true);
    }

    onFunctionCodeChanges();
}

void
MainWindow::
onWriteButtonPressed(bool enabled)
{
    if (enabled)
    {
        if (ui->radioButton_181->isChecked())
            ui->lineEdit_109->setReadOnly(false);
        else if (ui->radioButton_182->isChecked())
            ui->lineEdit_111->setReadOnly(false);
        else
            ui->groupBox_107->setEnabled(true);
    }

    onFunctionCodeChanges();
}

float 
MainWindow::
toFloat(QByteArray f)
{
    bool ok;
    int sign = 1;

    f = f.toHex(); // Convert to Hex

    qDebug() << "QByteArrayToFloat: QByteArray hex = " << f;

    f = QByteArray::number(f.toLongLong(&ok, 16), 2);    // Convert hex to binary

    if(f.length() == 32) {
        if(f.at(0) == '1') sign =-1;     // If bit 0 is 1 number is negative
        f.remove(0,1);                   // Remove sign bit
    }

    QByteArray fraction = f.right(23);  // Get the fractional part
    double mantissa = 0;
    for(int i = 0; i < fraction.length(); i++){  // Iterate through the array to claculate the fraction as a decimal.
        if(fraction.at(i) == '1')
            mantissa += 1.0 / (pow(2, i+1));
    }

    int exponent = f.left(f.length() - 23).toLongLong(&ok, 2) - 127;     // Calculate the exponent

    qDebug() << "QByteArrayToFloat: float number = "<< QString::number(sign * pow(2, exponent) * (mantissa + 1.0),'f', 5);

    return (sign * pow(2, exponent) * (mantissa + 1.0));
}

void
MainWindow::
updateRegisters(const bool isRazor, const int i)
{
    if (isRazor)
    {
        REG_SN_PIPE[i] = 1;
        REG_WATERCUT[i] = 3;
        REG_TEMPERATURE[i] = 5;
        REG_EMULSTION_PHASE[i] = 7;
        REG_SALINITY[i] = 9;
        REG_HARDWARE_VERSION[i] = 11;
        REG_FIRMWARE_VERSION[i] = 13;
        REG_OIL_ADJUST[i] = 15;
        REG_WATER_ADJUST[i] = 17;
        REG_FREQ[i] = 19;
        REG_FREQ_AVG[i] = 21;
        REG_WATERCUT_AVG[i] = 23;
        REG_WATERCUT_RAW[i] = 25;
        REG_ANALYZER_MODE[i] = 27;
        REG_TEMP_AVG[i] = 29;
        REG_TEMP_ADJUST[i] = 31;
        REG_TEMP_USER[i] = 33;
        REG_PROC_AVGING[i] = 35;
        REG_OIL_INDEX[i] = 37;
        REG_OIL_P0[i] = 39;
        REG_OIL_P1[i] = 41;
        REG_OIL_FREQ_LOW[i] = 43 ;
        REG_OIL_FREQ_HIGH[i] = 45;
        REG_SAMPLE_PERIOD[i] = 47;
        REG_AO_LRV[i] = 49;
        REG_AO_URV[i] = 51;
        REG_AO_DAMPEN[i] = 53;
        REG_BAUD_RATE[i] = 55;
        REG_SLAVE_ADDRESS[i] = 57;
        REG_STOP_BITS[i] = 59;
        REG_OIL_RP[i] = 61;
        REG_WATER_RP[i] = 63;
        REG_DENSITY_MODE[i] = 65;
        REG_OIL_CALC_MAX[i] = 67;
        REG_OIL_PHASE_CUTOFF[i] = 69;
        REG_TEMP_OIL_NUM_CURVES[i] = 71;
        REG_STREAM[i] = 73;
        REG_OIL_RP_AVG[i] = 75;
        REG_PLACE_HOLDER[i] = 77;
        REG_OIL_SAMPLE[i] = 79;
        REG_RTC_SEC[i] = 81;
        REG_RTC_MIN[i] = 83;
        REG_RTC_HR[i] = 85;
        REG_RTC_DAY[i] = 87;
        REG_RTC_MON[i] = 89;
        REG_RTC_YR[i] = 91;
        REG_RTC_SEC_IN[i] = 93;
        REG_RTC_MIN_IN[i] = 95;
        REG_RTC_HR_IN[i] = 97;
        REG_RTC_DAY_IN[i] = 99;
        REG_RTC_MON_IN[i] = 101;
        REG_RTC_YR_IN[i] = 103;
        REG_AO_MANUAL_VAL[i] = 105;
        REG_AO_TRIMLO[i] = 107;
        REG_AO_TRIMHI[i] = 109;
        REG_DENSITY_ADJ[i] = 111;
        REG_DENSITY_UNIT[i] = 113;
        REG_WC_ADJ_DENS[i] = 115;
        REG_DENSITY_D3[i] = 117;
        REG_DENSITY_D2[i] = 119;
        REG_DENSITY_D1[i] = 121;
        REG_DENSITY_D0[i] = 123;
        REG_DENSITY_CAL_VAL[i] = 125;
        REG_MODEL_CODE_0[i] = 127;
        REG_MODEL_CODE_1[i] = 129;
        REG_MODEL_CODE_2[i] = 131;
        REG_MODEL_CODE_3[i] = 133;
        REG_LOGGING_PERIOD[i] = 135;
        REG_PASSWORD[i] = 137;
        REG_STATISTICS[i] = 139;
        REG_ACTIVE_ERROR[i] = 141;
        REG_AO_ALARM_MODE[i] = 143;
        REG_AO_OUTPUT[i] = 145;
        REG_PHASE_HOLD_CYCLES[i] = 147;
        REG_RELAY_DELAY[i] = 149;
        REG_RELAY_SETPOINT[i] = 151;
        REG_AO_MODE[i] = 153;
        REG_OIL_DENSITY[i] = 155;
        REG_OIL_DENSITY_MODBUS[i] = 157;
        REG_OIL_DENSITY_AI[i] = 159;
        REG_OIL_DENSITY_MANUAL[i] = 161;
        REG_OIL_DENSITY_AI_LRV[i] = 163;
        REG_OIL_DENSITY_AI_URV[i] = 165;
        REG_OIL_DENS_CORR_MODE[i] = 167;
        REG_AI_TRIMLO[i] = 169;
        REG_AI_TRIMHI[i] = 171;
        REG_AI_MEASURE[i] = 173;
        REG_AI_TRIMMED[i] = 175;
    }
    else
    {
        REG_SN_PIPE[i] = 1;
        REG_WATERCUT[i] = 3;
        REG_TEMPERATURE[i] = 5;
        REG_EMULSTION_PHASE[i] = 7;
        REG_SALINITY[i] = 9;
        REG_HARDWARE_VERSION[i] = 11;
        REG_FIRMWARE_VERSION[i] = 13;
        REG_OIL_ADJUST[i] = 15;
        REG_WATER_ADJUST[i] = 17;
        REG_FREQ[i] = 19;
        REG_FREQ_AVG[i] = 21;
        REG_WATERCUT_AVG[i] = 23;
        REG_WATERCUT_RAW[i] = 25;
        REG_ANALYZER_MODE[i] = 27;
        REG_TEMP_AVG[i] = 29;
        REG_TEMP_ADJUST[i] = 31;
        REG_TEMP_USER[i] = 33;
        REG_PROC_AVGING[i] = 35;
        REG_OIL_INDEX[i] = 37;
        REG_OIL_P0[i] = 39;
        REG_OIL_P1[i] = 41;
        REG_OIL_FREQ_LOW[i] = 43 ;
        REG_OIL_FREQ_HIGH[i] = 45;
        REG_SAMPLE_PERIOD[i] = 47;
        REG_AO_LRV[i] = 49;
        REG_AO_URV[i] = 51;
        REG_AO_DAMPEN[i] = 53;
        REG_BAUD_RATE[i] = 55;
        REG_SLAVE_ADDRESS[i] = 57;
        REG_STOP_BITS[i] = 59;
        REG_OIL_RP[i] = 61;
        REG_WATER_RP[i] = 63;
        REG_DENSITY_MODE[i] = 65;
        REG_OIL_CALC_MAX[i] = 67;
        REG_OIL_PHASE_CUTOFF[i] = 69;
        REG_TEMP_OIL_NUM_CURVES[i] = 71;
        REG_STREAM[i] = 73;
        REG_OIL_RP_AVG[i] = 75;
        REG_PLACE_HOLDER[i] = 77;
        REG_OIL_SAMPLE[i] = 79;
        REG_RTC_SEC[i] = 81;
        REG_RTC_MIN[i] = 83;
        REG_RTC_HR[i] = 85;
        REG_RTC_DAY[i] = 87;
        REG_RTC_MON[i] = 89;
        REG_RTC_YR[i] = 91;
        REG_RTC_SEC_IN[i] = 93;
        REG_RTC_MIN_IN[i] = 95;
        REG_RTC_HR_IN[i] = 97;
        REG_RTC_DAY_IN[i] = 99;
        REG_RTC_MON_IN[i] = 101;
        REG_RTC_YR_IN[i] = 103;
        REG_AO_MANUAL_VAL[i] = 105;
        REG_AO_TRIMLO[i] = 107;
        REG_AO_TRIMHI[i] = 109;
        REG_DENSITY_ADJ[i] = 111;
        REG_DENSITY_UNIT[i] = 113;
        REG_WC_ADJ_DENS[i] = 115;
        REG_DENSITY_D3[i] = 117;
        REG_DENSITY_D2[i] = 119;
        REG_DENSITY_D1[i] = 121;
        REG_DENSITY_D0[i] = 123;
        REG_DENSITY_CAL_VAL[i] = 125;
        REG_MODEL_CODE_0[i] = 127;
        REG_MODEL_CODE_1[i] = 129;
        REG_MODEL_CODE_2[i] = 131;
        REG_MODEL_CODE_3[i] = 133;
        REG_LOGGING_PERIOD[i] = 135;
        REG_PASSWORD[i] = 137;
        REG_STATISTICS[i] = 139;
        REG_ACTIVE_ERROR[i] = 141;
        REG_AO_ALARM_MODE[i] = 143;
        REG_AO_OUTPUT[i] = 145;
        REG_PHASE_HOLD_CYCLES[i] = 147;
        REG_RELAY_DELAY[i] = 149;
        REG_RELAY_SETPOINT[i] = 151;
        REG_AO_MODE[i] = 153;
        REG_OIL_DENSITY[i] = 155;
        REG_OIL_DENSITY_MODBUS[i] = 157;
        REG_OIL_DENSITY_AI[i] = 159;
        REG_OIL_DENSITY_MANUAL[i] = 161;
        REG_OIL_DENSITY_AI_LRV[i] = 163;
        REG_OIL_DENSITY_AI_URV[i] = 165;
        REG_OIL_DENS_CORR_MODE[i] = 167;
        REG_AI_TRIMLO[i] = 169;
        REG_AI_TRIMHI[i] = 171;
        REG_AI_MEASURE[i] = 173;
        REG_AI_TRIMMED[i] = 175;
    }
}
