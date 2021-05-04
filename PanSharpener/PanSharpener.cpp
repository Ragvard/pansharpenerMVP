#include "PanSharpener.h"
#include <iostream>
#include <set>
#include <filesystem>
#include "Brovey.h"
#include "SimpleMean.h"
#include "Combine.h"
#include "IHSSharpening.h"
#include "Maximize.h"
#include "GramSchmidt.h"
#include <QMessageBox>
#include <QToolButton>
#include <QCloseEvent>


namespace fs = std::filesystem;

PanSharpener::PanSharpener(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
    this->setWindowIcon(QIcon("icon.ico"));
    images = { NULL, NULL, NULL, NULL, NULL};
    paths = { "", "", "", "", "", "" };
    imageSlots = { ui.GroupBoxOne, ui.GroupBoxTwo, ui.GroupBoxThree, ui.GroupBoxFour, ui.GroupBoxPan };
    loadButtons = { ui.LoadOne, ui.LoadTwo, ui.LoadThree, ui.LoadFour, ui.LoadPan };
    clearButtons = { ui.ClearOne, ui.ClearTwo, ui.ClearThree, ui.ClearFour, ui.ClearPan };
    imageInfos = { {ui.PreviewOne, ui.XSizeOne, ui.YSizeOne,}, {ui.PreviewTwo, ui.XSizeTwo, ui.YSizeTwo},
        {ui.PreviewThree, ui.XSizeThree, ui.YSizeThree}, {ui.PreviewFour, ui.XSizeFour, ui.YSizeFour}, {ui.PreviewPan, ui.XSizePan, ui.YSizePan} };
    addDataLabels = { ui.addLabelOne, ui.addLabelTwo, ui.addLabelThree, ui.addLabelFour,
                                    ui.addLabelFive, ui.addLabelSix, ui.addLabelSeven, ui.addLabelEight };
    addSpinBoxes = { ui.addSpinBoxOne, ui.addSpinBoxTwo, ui.addSpinBoxThree, ui.addSpinBoxFour,
                                    ui.addSpinBoxFive, ui.addSpinBoxSix, ui.addSpinBoxSeven, ui.addSpinBoxEight };
    resampleTypes = { ui.RescaleTypeOne, ui.RescaleTypeTwo, ui.RescaleTypeThree };
    rescaleButtons = { ui.RescaleOne, ui.RescaleTwo, ui.RescaleThree, ui.RescaleFour };
    otherButtons = { ui.mergeButton, ui.clearButton, ui.directoryPushButtonOpen, ui.directoryPushButtonSave, ui.closeButton };
    imagePictures = {ui.PreviewImageOne, ui.PreviewImageTwo, ui.PreviewImageThree, ui.PreviewImageFour, ui.PreviewImagePan };
    ui.modelComboBox->addItems({"Combine", "Simple Mean",  "Maximze", "Brovey", "Brovey + NIR", "IHS Transfrom", "IHS Transfrom + NIR", "Gram-Schmidt (WIP)" });
    oldAlgNumber = -1;
    ui.horizontalLayout_7->setAlignment(Qt::AlignLeft);
    ui.horizontalLayout_9->setAlignment(Qt::AlignLeft);
    ui.horizontalLayout_11->setAlignment(Qt::AlignLeft);
    ui.horizontalLayout_12->setAlignment(Qt::AlignLeft);
    ui.horizontalLayout_7->setAlignment(Qt::AlignTop);
    ui.horizontalLayout_9->setAlignment(Qt::AlignTop);
    ui.horizontalLayout_11->setAlignment(Qt::AlignTop);
    ui.horizontalLayout_12->setAlignment(Qt::AlignTop);
    ui.verticalLayout->setAlignment(Qt::AlignTop);
    for (QPushButton* it : rescaleButtons) it->setDisabled(true);
    for (QLabel* it : addDataLabels) it->setAlignment(Qt::AlignLeft);
    for (QDoubleSpinBox* it : addSpinBoxes) it->setAlignment(Qt::AlignLeft);    
    baseOpenDirectory = QDir::homePath() + "//Desktop";
    baseSaveDirectory = QDir::homePath() + "//Desktop";
    for (QLabel* it : imagePictures) it->clear();
    verifyConditions();
    
}

////////////

void PanSharpener::closeEvent(QCloseEvent* event)
{
    QMessageBox reply;
    reply.setText("Close PanSharpener?");
    reply.setWindowTitle("Confirmation");
    reply.setStandardButtons(QMessageBox::Yes);
    reply.addButton(QMessageBox::No);
    reply.setDefaultButton(QMessageBox::No);
    reply.setIcon(QMessageBox::Icon::Question);
    reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (QMessageBox::StandardButton::Yes == reply.exec()) event->accept();
    else event->ignore();
}


void PanSharpener::AlgChange(int n)
{
    verifyConditions();
}

void PanSharpener::ChangeOpenDirectory()
{
    QString n = QFileDialog::getExistingDirectory(this, "Choose default image directory", QDir::homePath() + "//Desktop");
    if (n != "") baseOpenDirectory = n;
   
}
void PanSharpener::ChangeSaveDirectory()
{
    QString n = QFileDialog::getExistingDirectory(this, "Choose default save directory", QDir::homePath() + "//Desktop");
    if (n != "") baseSaveDirectory = n;
}

QString getSavePath(QWidget* parent, QString message, QString baseSaveDirectory)
{
    QString filter("GeoTiff files (*.tif *.tiff)");
    return QFileDialog::getSaveFileName(parent, message, baseSaveDirectory, filter);
}
QString getOpenPath(QWidget* parent, QString filter, QString baseDirectory)
{
    return QFileDialog::getOpenFileName(parent, "Open a file", baseDirectory, filter);
}

int PanSharpener::getSlotNumber(QGroupBox* slot)
{
    int i;
    for (i = 0; i < 5; i++) if (slot == imageSlots[i]) break;
    return i;
}

void PanSharpener::showImageSlot(int i, bool b) 
{
    if (b) imageSlots[i]->show();
    else imageSlots[i]->hide();
}
void PanSharpener::showImageSlot(QGroupBox* slot, bool b)
{
    if (b) slot->show();
    else slot->hide();
}

void PanSharpener::clearImageSlot(QGroupBox* slot) { clearImageSlot(getSlotNumber(slot)); }
void PanSharpener::clearImageSlot(int i)
{
    imageInfos[i][0]->setText("No image selected.");
    imageInfos[i][1]->setText("xSize:");
    imageInfos[i][2]->setText("ySize:");
    images[i] = NULL;
    sizes[i][0] = 0;
    sizes[i][1] = 0;
    paths[i] = "";
    imagePictures[i]->clear();
    if (i < 4) rescaleButtons[i]->setDisabled(true);
    verifyConditions();
}

void PanSharpener::lockImageSlot(QGroupBox* slot, bool b) { slot->setDisabled(b); }
void PanSharpener::lockImageSlot(int i, bool b) { imageSlots[i]->setDisabled(b); }

void PanSharpener::loadImageSlot(QGroupBox* slot) { loadImageSlot(getSlotNumber(slot), ""); }
void PanSharpener::loadImageSlot(int i, string path)
{

    string imagePath = path;
    if (imagePath == "") imagePath = getOpenPath(this, "GeoTiff files (*.tif *.tiff)", baseOpenDirectory).toStdString();
    if (imagePath == "") return;
    
    GDALDataset* newFile = (GDALDataset*)GDALOpen(imagePath.c_str(), GA_ReadOnly);
    if (newFile == NULL)
    {
        QMessageBox reply;
        reply.setText("Unable to open file.");
        reply.setWindowTitle("Error");
        reply.setIcon(QMessageBox::Icon::Critical);
        reply.setStandardButtons(QMessageBox::Ok);
        reply.setDefaultButton(QMessageBox::Ok);
        reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        reply.exec();
        return;
    }

    images[i] = newFile;
    sizes[i][0] = images[i]->GetRasterXSize();
    sizes[i][1] = images[i]->GetRasterYSize();
    imageInfos[i][0]->setText(QString::fromStdString(fs::path(imagePath).filename().string()));
    imageInfos[i][1]->setText(QString::fromStdString("xSize: " + to_string(sizes[i][0])));
    imageInfos[i][2]->setText(QString::fromStdString("ySize: " + to_string(sizes[i][0])));
    if (i < 4) rescaleButtons[i]->setDisabled(false);
    paths[i] = imagePath;

    GDALRasterBand* band = images[i]->GetRasterBand(1);
    QImage preview(100, 100, QImage::Format_Grayscale16);
    float* line = new float[sizes[i][0]];
    int step1 = sizes[i][0] / 100;
    int step2 = sizes[i][1] / 100;
    
    for (int k = 0; k < 100; k++)
    {
        band->RasterIO(GF_Read, 0, k * step2, sizes[i][0], 1, line, sizes[i][0], 1, GDT_Float32, 0, 0);
        for (int j = 0; j < 100; j ++)
        {
            int copy = (uint)(line[j * step1] / 65536 * 255);
            int transp = copy == 0 ? 0 : 255;
            preview.setPixel(k, j, qRgba(copy, copy, copy, transp));
        }
    }
    imagePictures[i]->setPixmap(QPixmap::fromImage(preview));
    delete[] line;
    verifyConditions();
}

void PanSharpener::showAddData(int i, bool b)
{
    if (b)
    {
        addDataLabels[i]->show();
        addSpinBoxes[i]->show();
    }
    else
    {
        addDataLabels[i]->hide();
        addSpinBoxes[i]->hide();
    }
}

void PanSharpener::clearAddData(int i)
{
    addDataLabels[i]->setText("Add. data " + (i + 1));
    addSpinBoxes[i]->setSingleStep(1);
    addSpinBoxes[i]->setMinimum(0);
    addSpinBoxes[i]->setMaximum(100);
    addSpinBoxes[i]->setValue(0);
}
   
void PanSharpener::setAddData(int i, string name, double step, double min, double max, double start)
{
    addDataLabels[i]->setText(QString::fromStdString(name));
    addDataLabels[i]->show();
    addSpinBoxes[i]->setValue(start);
    addSpinBoxes[i]->setSingleStep(step);
    addSpinBoxes[i]->setMinimum(min);
    addSpinBoxes[i]->setMaximum(max);
    addSpinBoxes[i]->show();
}

void PanSharpener::hideFurtherAddData(int i) { for (i; i < 8; i++) showAddData(i, false); }

void showRescales(vector<QPushButton*> vec, bool b) 
{
    if (b) for (QPushButton* it : vec) it->show();
    else for (QPushButton* it : vec) it->hide();
}

void PanSharpener::verifyConditions()
{
    int i = ui.modelComboBox->currentIndex();
    switch (i)
    {
    case 0: // Combine
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, false);
            if (images[4] != NULL) clearImageSlot(4);
            showRescales(rescaleButtons, false);

            ui.GroupBoxRescale->hide();
            ui.GroupBoxAdd->hide();

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 2; i++)
        {
            if (sizes[i][0] != sizes[2][0] || sizes[i][1] != sizes[2][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();
        break;
    }
    case 1: // Mean
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->show();

            setAddData(0, "Red", 0.05, 0, 2, 1);
            setAddData(1, "Green", 0.05, 0, 2, 1);
            setAddData(2, "Blue", 0.05, 0, 2, 1);
            hideFurtherAddData(3);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 3; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();
        break;
    }
    case 2: // Maximize
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->show();
            setAddData(0, "Red", 0.05, 0, 2, 1);
            setAddData(1, "Green", 0.05, 0, 2, 1);
            setAddData(2, "Blue", 0.05, 0, 2, 1);
            hideFurtherAddData(3);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 3; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();
        break;
    }
    case 3: // Brovey
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->show();
            setAddData(0, "Red", 0.05, 0, 2, 1);
            setAddData(1, "Green", 0.05, 0, 2, 1);
            setAddData(2, "Blue", 0.05, 0, 2, 1);
            hideFurtherAddData(3);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;

        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 3; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();
        break;
    }
    case 4: // Brovey + NIR
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, true);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->show();
            setAddData(0, "Red weight", 0.01, 0, 1, 1);
            setAddData(1, "Green weight", 0.01, 0, 1, 1);
            setAddData(2, "Blue weight", 0.01, 0, 1, 1);
            setAddData(3, "Infrared weight", 0.01, 0, 1, 0);
            hideFurtherAddData(4);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
            imageSlots[3]->setTitle("Infrared band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[3] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 4; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();
        break;
    }
    case 5: // IHS
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->hide();
            hideFurtherAddData(0);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 3; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();

        break;
    }
    case 6: // IHS + NIR
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, true);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->show();
            setAddData(0, "Infrared weight", 0.01, 0, 1, 0);
            hideFurtherAddData(1);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[3] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 4; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();

        break;
    }
    case 7: // Gram-Schmidt ------------------------------------------- WIP
    {
        if (i != oldAlgNumber)
        {
            showImageSlot(0, true);
            showImageSlot(1, true);
            showImageSlot(2, true);
            showImageSlot(3, false);
            if (images[3] != NULL) clearImageSlot(3);
            showImageSlot(4, true);
            showRescales(rescaleButtons, true);

            ui.GroupBoxRescale->show();
            ui.GroupBoxAdd->hide();
            hideFurtherAddData(0);

            imageSlots[0]->setTitle("Red band");
            imageSlots[1]->setTitle("Green band");
            imageSlots[2]->setTitle("Blue band");
        }
        oldAlgNumber = i;


        if (images[0] == NULL || images[1] == NULL || images[2] == NULL || images[4] == NULL)
        {
            lockMerge("Choose all images.");
            return;
        }
        else for (int i = 0; i < 3; i++)
        {
            if (sizes[i][0] != sizes[4][0] || sizes[i][1] != sizes[4][1])
            {
                lockMerge("Image resolutions are not the same.");
                return;
            }
        }

        unlockMerge();

        break;
    }
    default:
        break;

    }
}

void PanSharpener::lockMerge(string reason)
{
    ui.mergeButton->setDisabled(true);
    ui.mergeStatus->setText(QString::fromStdString(reason));
}

void PanSharpener::unlockMerge()
{
    ui.mergeButton->setDisabled(false);
    ui.mergeStatus->setText(QString::fromStdString("You can execute algorithm."));
}

void PanSharpener::lockChanges(string message)
{
    for (QGroupBox* it : imageSlots) lockImageSlot(it, true);
    for (QDoubleSpinBox* it : addSpinBoxes) it->setDisabled(true);
    for (QRadioButton* it : resampleTypes) it->setDisabled(true);
    for (QPushButton* it : rescaleButtons) it->setDisabled(true);
    for (QPushButton* it : otherButtons) it->setDisabled(true);
    ui.modelComboBox->setDisabled(true);
    
    ui.mergeStatus->setText(QString::fromStdString(message));

}

void PanSharpener::unlockChanges()
{
    for (QGroupBox* it : imageSlots) lockImageSlot(it, false);
    for (QDoubleSpinBox* it : addSpinBoxes) it->setDisabled(false);
    for (QRadioButton* it : resampleTypes) it->setDisabled(false);
    for (int i = 0; i < 4; i++) if (images[i] != NULL) rescaleButtons[i]->setDisabled(false);
    for (QPushButton* it : otherButtons) it->setDisabled(false);
    ui.modelComboBox->setDisabled(false);
    verifyConditions();
}

extern void startAlg(vector<GDALDataset*> inp, string filename, PansharpAlg* alg, vector<float> addData)
{
    alg->applySharpening(inp, filename, addData);
}

void PanSharpener::handleFinishedMerge()
{
    delete watcher;
    delete alg;
    unlockChanges();
}

void PanSharpener::Merge()
{
    QMessageBox reply;
    reply.setText("Are you sure you want to start image merge? This might take a while.");
    reply.setWindowTitle("Confirmation");
    reply.setStandardButtons(QMessageBox::Yes);
    reply.addButton(QMessageBox::No);
    reply.setDefaultButton(QMessageBox::No);
    reply.setIcon(QMessageBox::Icon::Question);
    reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (reply.exec() == QMessageBox::No) return;

    QString filename = getSavePath(this, "Select new file name", baseSaveDirectory);
    if (filename == "") return;

    if (!removeFile(filename.toStdString()))
    {
        QMessageBox reply;
        fs::path p = fs::path(filename.toStdString());
        QString message = QString::fromStdString("Can't update existing file " + p.stem().string() + "rescaled" + p.extension().string() + ". Probably it is being used somewhere else.");
        reply.setText(message);
        reply.setWindowTitle("Error");
        reply.setStandardButtons(QMessageBox::Ok);
        reply.setDefaultButton(QMessageBox::Ok);
        reply.setIcon(QMessageBox::Icon::Warning);
        reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        reply.exec();
        return;
    }

    watcher = new QFutureWatcher<void>();
    QFuture<void> future;
    connect(watcher, SIGNAL(finished()), this, SLOT(handleFinishedMerge()));
    lockChanges("Merging in progress. Please, do not close the application window.");

    switch (int i = ui.modelComboBox->currentIndex())
    {
    case 0: // Combine
    {
        alg = new Combine();;
        vector<float> addData;
        for (int i = 0; i < 3; i++) addData.push_back(addSpinBoxes[i]->value());
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }

    case 1: // Mean
    {
        vector<float> addData;
        for (int i = 0; i < 3; i++) addData.push_back(addSpinBoxes[i]->value());
        alg = new SimpleMean();
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }
    case 2: // Maximize
    {
        vector<float> addData;
        for (int i = 0; i < 3; i++) addData.push_back(addSpinBoxes[i]->value());
        alg = new Maximize();
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }
    case 3: // Brovey
    {
        vector<float> addData;
        for (int i = 0; i < 3; i++) addData.push_back(addSpinBoxes[i]->value());
        alg = new Brovey();
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }
    case 4: // Brovey + NIR
    {
        alg = new Brovey();
        vector<float> addData;
        for (int i = 0; i < 4; i++) addData.push_back(addSpinBoxes[i]->value());
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }
    case 5: // IHS
    {
        alg = new IHSSharpening();
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, vector<float>());
        watcher->setFuture(future);
        break;
    }
    case 6: // IHS + NIR
    {
        alg = new IHSSharpening();
        vector<float> addData;
        for (int i = 0; i < 1; i++) addData.push_back(addSpinBoxes[i]->value());
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, addData);
        watcher->setFuture(future);
        break;
    }
    case 7: // Gram-Schmidt ------------------------------------------- WIP
    {
        alg = new GramSchmidt();
        future = QtConcurrent::run(startAlg, images, filename.toStdString(), alg, vector<float>());
        watcher->setFuture(future);
        break;
    }
    default:
        break;

    }
}

void PanSharpener::LoadImageOne() { loadImageSlot(0, ""); }
void PanSharpener::LoadImageTwo() { loadImageSlot(1, ""); }
void PanSharpener::LoadImageThree() { loadImageSlot(2, ""); }
void PanSharpener::LoadImageFour() { loadImageSlot(3, ""); }
void PanSharpener::LoadImagePan() { loadImageSlot(4, ""); }


void PanSharpener::ClearImageOne() { clearImageSlot(0); }
void PanSharpener::ClearImageTwo() { clearImageSlot(1); }
void PanSharpener::ClearImageThree() { clearImageSlot(2); }
void PanSharpener::ClearImageFour() { clearImageSlot(3); }
void PanSharpener::ClearImagePan() { clearImageSlot(4); }
void PanSharpener::ClearAllInput()
{
    QMessageBox reply;
    reply.setText("Are you sure you want to clear all inputs?");
    reply.setWindowTitle("Confirmation");
    reply.setStandardButtons(QMessageBox::Yes);
    reply.addButton(QMessageBox::No);
    reply.setDefaultButton(QMessageBox::No);
    reply.setIcon(QMessageBox::Icon::Question);
    reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (reply.exec() == QMessageBox::No) return;
    for (QGroupBox* it : imageSlots)  clearImageSlot(it);
}

void PanSharpener::handleFinishedRescale()
{
    delete watcher;
    loadImageSlot(rescaledImageSlot, paths[rescaledImageSlot]);
    unlockChanges();
}
void PanSharpener::rescaleImage(int i)
{

    if (images[4] == NULL)
    {
        QMessageBox reply;
        reply.setText("You need to choose high-resolution image first.");
        reply.setWindowTitle("Warning");
        reply.setStandardButtons(QMessageBox::Ok);
        reply.setDefaultButton(QMessageBox::Ok);
        reply.setIcon(QMessageBox::Icon::Warning);
        reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        reply.exec();
        return;
    }
    else if ((sizes[i][0] == sizes[4][0]) && (sizes[i][1] == sizes[4][1]))
    {
        QMessageBox reply;
        reply.setText("There is no need for rescaling.");
        reply.setWindowTitle("Warning");
        reply.setStandardButtons(QMessageBox::Ok);
        reply.setDefaultButton(QMessageBox::Ok);
        reply.setIcon(QMessageBox::Icon::Warning);
        reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        reply.exec();
        return;
    }

    GDALResampleAlg alg = GRA_NearestNeighbour;
    if (ui.RescaleTypeTwo->isChecked()) alg = GRA_Bilinear;
    else if (ui.RescaleTypeThree->isChecked()) alg = GRA_Cubic;

    if (images[i] != NULL) // Fool-proof!
    {
        fs::path p = fs::path(paths[i]);
        string newpath = p.parent_path().string() + "/" + p.stem().string() + "rescaled" + p.extension().string();
        if (!removeFile(newpath))
        {
            QMessageBox reply;
            QString message = QString::fromStdString("Can't update existing file " + p.stem().string() + "rescaled" + p.extension().string() + ". Probably it is being used somewhere else.");
            reply.setText(message);
            reply.setWindowTitle("Error");
            reply.setStandardButtons(QMessageBox::Ok);
            reply.setDefaultButton(QMessageBox::Ok);
            reply.setIcon(QMessageBox::Icon::Warning);
            reply.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            reply.exec();
            return;
        }

        lockChanges("Image " + p.filename().string() + " is rescaling");
        paths[i] = newpath;
        watcher = new QFutureWatcher<void>();
        rescaledImageSlot = i;
        QFuture<void> future;
        connect(watcher, SIGNAL(finished()), this, SLOT(handleFinishedRescale()));
        future = QtConcurrent::run(PansharpAlg::rescale, images[i], images[4], alg, newpath);
        watcher->setFuture(future);
    }

}

bool PanSharpener::removeFile(string path)
{
    QFile file(QString::fromStdString(path));
    if (file.open(QIODevice::ReadWrite) && !file.remove()) return false;
    else return true;
    
}

void PanSharpener::RescaleImageOne() { rescaleImage(0); }
void PanSharpener::RescaleImageTwo() { rescaleImage(1); }
void PanSharpener::RescaleImageThree() { rescaleImage(2); }
void PanSharpener::RescaleImageFour() { rescaleImage(3); }