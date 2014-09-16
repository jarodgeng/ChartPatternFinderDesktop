#include "MainWindow.h"
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include "plot.h"

#include <QTableView>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include "PatternMatchTableView.h"
#include <QFileDialog>

#include "PeriodValSegment.h"
#include "DoubleBottomScanner.h"
#include "PatternShapeGenerator.h"
#include "MultiPatternScanner.h"
#include "PatternMatchFilter.h"
#include "SymetricTriangleScanner.h"
#include "DescendingTriangleScanner.h"
#include "AscendingTriangleScanner.h"
#include "CupWithHandleScanner.h"
#include "CupWithoutHandleScanner.h"
#include "RectangleScanner.h"
#include "PivotHighScanner.h"
#include "RisingWedgeScanner.h"
#include "FallingWedgeScanner.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    QToolBar *toolBar = new QToolBar( this );

    QComboBox *typeBox = new QComboBox( toolBar );
    typeBox->addItem( "Bars" );
    typeBox->addItem( "CandleSticks" );
    typeBox->setCurrentIndex( 1 );
    typeBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    QToolButton *btnExport = new QToolButton( toolBar );
    btnExport->setText( "Export" );
    btnExport->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
 //   connect( btnExport, SIGNAL( clicked() ), d_plot, SLOT( exportPlot() ) );

    toolBar->addWidget( typeBox );
    toolBar->addWidget( btnExport );
    addToolBar( toolBar );

    patternTable_ = new PatternMatchTableView();
    instrumentListTableView_ = new InstrumentListTableView();

    d_plot = new Plot( this );
    d_plot->setMode( typeBox->currentIndex() );
    connect( typeBox, SIGNAL( currentIndexChanged( int ) ),
        d_plot, SLOT( setMode( int ) ) );

    QGridLayout *mainWindowGridLayout = new QGridLayout;
    mainWindowGridLayout->addWidget(instrumentListTableView_,0,0,2,1);
    mainWindowGridLayout->addWidget(d_plot,0,1,1,1);
    mainWindowGridLayout->addWidget(patternTable_,1,1,1,1);
    mainWindowGridLayout->setColumnStretch(0, 10);
    mainWindowGridLayout->setColumnStretch(1, 30);
    mainWindowGridLayout->setRowStretch(0,30);
    mainWindowGridLayout->setRowStretch(1,10);


    QWidget *centralWindow = new QWidget();
    centralWindow->setLayout(mainWindowGridLayout);
    setCentralWidget(centralWindow);


    QString quoteFileDirName = QFileDialog::getExistingDirectory(this, "Select Quotes Directory");
    if(!quoteFileDirName.isNull()) {
        instrumentListTableView_->populateFromCSVFiles(quoteFileDirName);
    }

    // Layout is finished. Populate the pattern plot and pattern selectin table with some data.

    connect(patternTable_, SIGNAL(patternMatchSelected (const PatternMatchPtr &)),
              this, SLOT(patternMatchSelected(const PatternMatchPtr &)));

    connect(instrumentListTableView_, SIGNAL(instrumentSelected (const QString &)),
              this, SLOT(instrumentSelected(const QString &)));

}

MainWindow::~MainWindow()
{
}

void MainWindow::patternMatchSelected(const PatternMatchPtr &selectedMatch)
{
    d_plot->populatePatternShapes(selectedMatch);

}

void MainWindow::instrumentSelected(const QString &instumentFilePath)
{
    qDebug() << "MainWindow: Instrument selected: " << instumentFilePath;

    PeriodValSegmentPtr chartData = PeriodValSegment::readFromFile(instumentFilePath.toStdString());

    PatternMatchListPtr currentPatternMatches = PatternMatchListPtr(new PatternMatchList());

    PeriodValCltnIterListPtr pivotHighBeginIters = PivotHighScanner().scanPivotHighBeginIters(chartData);


     PatternScannerPtr doubleBottomScanner(new DoubleBottomScanner(DoubleRange(7.0,40.0)));
     MultiPatternScanner multiScanner(doubleBottomScanner);
     PatternMatchListPtr doubleBottoms = multiScanner.scanUniquePatternMatches(chartData,pivotHighBeginIters);
     currentPatternMatches->insert(currentPatternMatches->end(),doubleBottoms->begin(),doubleBottoms->end());

     SymetricTriangleScanner wedgeScanner;
     PatternMatchListPtr symetricTriangles = wedgeScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),symetricTriangles->begin(),symetricTriangles->end());

     DescendingTriangleScanner descTriangleScanner;
     PatternMatchListPtr descTriangle = descTriangleScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),descTriangle->begin(),descTriangle->end());

     RectangleScanner flatBaseScanner;
     PatternMatchListPtr flatBases = flatBaseScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),flatBases->begin(),flatBases->end());

     RisingWedgeScanner risingWedgeScanner;
     PatternMatchListPtr risingWedges = risingWedgeScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),risingWedges->begin(),risingWedges->end());

     FallingWedgeScanner fallingWedgeScanner;
     PatternMatchListPtr fallingWedges = fallingWedgeScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),fallingWedges->begin(),fallingWedges->end());

     AscendingTriangleScanner ascendingTriangleScanner;
     PatternMatchListPtr ascTriangles = ascendingTriangleScanner.scanPatternMatches(chartData);
     currentPatternMatches->insert(currentPatternMatches->end(),ascTriangles->begin(),ascTriangles->end());

     PatternScannerPtr cupWithoutHandleScanner(new CupWithoutHandleScanner());
     MultiPatternScanner multiCupScanner(cupWithoutHandleScanner);
     PatternMatchListPtr cupWithoutHandleMatches = multiCupScanner.scanUniquePatternMatches(chartData,pivotHighBeginIters);
     currentPatternMatches->insert(currentPatternMatches->end(),cupWithoutHandleMatches->begin(),cupWithoutHandleMatches->end());

      PatternScannerPtr cupWithHandleScanner(new CupWithHandleScanner());
     MultiPatternScanner multiCupWithHandleScanner(cupWithHandleScanner);
     PatternMatchListPtr cupWithHandleMatches = multiCupWithHandleScanner.scanUniquePatternMatches(chartData,pivotHighBeginIters);
     currentPatternMatches->insert(currentPatternMatches->end(),cupWithHandleMatches->begin(),cupWithHandleMatches->end());

     d_plot->populateChartData(chartData);

     patternTable_->populatePatternMatches(currentPatternMatches);

     if(patternTable_->currentPatternMatches()->size() > 0)
     {
         patternTable_->selectRow(0);
         d_plot->populatePatternShapes(patternTable_->currentPatternMatches()->front());
     }

}
