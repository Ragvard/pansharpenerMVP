#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PanSharpener.h"
#include <QFileDialog>
#include <gdal_priv.h>
#include "PansharpAlg.h"
#include <QtConcurrent/QtConcurrent>
#include <queue>

using namespace std;


class PanSharpener : public QMainWindow
{
	Q_OBJECT

public:

	PanSharpener(QWidget* parent = Q_NULLPTR);

private slots:
	void closeEvent(QCloseEvent* event);
	void Merge();
	void AlgChange(int n);
	void LoadImageOne();
	void LoadImageTwo();
	void LoadImageThree();
	void LoadImageFour();
	void LoadImagePan();
	void ClearImageOne();
	void ClearImageTwo();
	void ClearImageThree();
	void ClearImageFour();
	void ClearImagePan();
	void ClearAllInput();
	void RescaleImageOne();
	void RescaleImageTwo();
	void RescaleImageThree();
	void RescaleImageFour();
	void handleFinishedMerge();
	void handleFinishedRescale();
	void ChangeOpenDirectory();
	void ChangeSaveDirectory();


private:
	Ui::PanSharpenerClass ui;

	void verifyConditions();

	void lockMerge(string reason);
	void unlockMerge();
	void lockChanges(string message);
	void unlockChanges();

	void showImageSlot(int i, bool b);
	void showImageSlot(QGroupBox* slot, bool b);

	void clearImageSlot(int i);
	void clearImageSlot(QGroupBox* slot);

	void lockImageSlot(int i, bool b);
	void lockImageSlot(QGroupBox* slot, bool b);

	void loadImageSlot(int i, string path);
	void loadImageSlot(QGroupBox* slot);

	int getSlotNumber(QGroupBox* slot);

	void rescaleImage(int i);
	
	void showAddData(int i, bool b);
	void clearAddData(int i);
	void setAddData(int i, string name, double step, double min, double max, double start);
	void hideFurtherAddData(int i);

	bool removeFile(string path);

	QFutureWatcher<void>* watcher;
	PansharpAlg* alg;
	int oldAlgNumber;
	int rescaledImageSlot;
	int sizes[5][2];
	QString baseOpenDirectory;
	QString baseSaveDirectory;

	vector<string> paths;
	vector<GDALDataset*> images;
	vector<QGroupBox*> imageSlots;
	vector<QPushButton*> loadButtons;
	vector<QPushButton*> clearButtons;
	vector<vector<QLabel*>> imageInfos;
	vector<QLabel*> imagePictures;
	vector<QRadioButton*> resampleTypes;
	vector<QPushButton*> rescaleButtons;
	vector<QLabel*> addDataLabels;
	vector<QDoubleSpinBox*> addSpinBoxes;
	vector<QPushButton*> otherButtons;
};
