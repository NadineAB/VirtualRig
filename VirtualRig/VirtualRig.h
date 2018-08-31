#ifndef VIRTUALRIG_H
#define VIRTUALRIG_H
/* \brief:
QT GUI interface
*/
#include <QtGui/QMainWindow>
#include "ui_VirtualRig.h"

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

class GLArea;

class VirtualRig : public QMainWindow
{
	Q_OBJECT

public:
	VirtualRig(QWidget *parent = 0, Qt::WFlags flags = 0);
	~VirtualRig();

	public slots:
		void triggered(QAction *);

		void xscaleChanged(double);
		void yscaleChanged(double);
		void zscaleChanged(double);

		void xtransChanged(double);
		void ytransChanged(double);
		void ztransChanged(double);

		void PlayAnimationClick(bool);
		void PlayClick(bool);
		void PauseClick(bool);
		//void OneStepClick(bool);
		//void StopClick(bool);
		void heatWeightsClick(bool);
		void nearestBoneWeightsClick(bool);
		void smoothWeightsClick(bool);

		void doubleClicked(const QModelIndex &);
		void onBvhLoaded();
		void onMeshLoaded();
		void InitBVHFileCom();
		void selectModelFromCombobox(int newIndex);

		void onSkeletonToggled(bool);
		void onBvhVisibleToggled(bool);
		void onWireVisibleToggled(bool);
		void onErrorVisibleToggled(bool);
		void LBSClick(bool);
		void SaveObjClick(bool);

protected:
	void keyPressEvent(QKeyEvent *event);

private:
	Ui::virtuaClass ui;

	QSlider * createSlider();

	GLArea * glArea;

	QSlider * shinSlider;
	QSlider * ySlider;
	QSlider * zSlider;

	void InitBvhFileList();
	void InitMeshFileList();
};

#endif // VIRTUALRIG_H
