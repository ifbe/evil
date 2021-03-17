#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>
#include <QtWidgets/QPushButton>
#include <QtGui/qpainter.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define _hash_ hex32('h','a','s','h')




extern "C"{
struct pernode
{
	u64 type;
	u64 addr;
	u8 str[16];
};
struct perwire
{
	u16 src;
	u16 dst;
};
struct vert2d
{
        float x;
        float y;
};
void forcedirected_2d(
        struct vert2d* obuf, int olen,
        struct vert2d* vbuf, int vlen,
        struct perwire* lbuf, int llen);
}




class MyMainWindow : public QMainWindow
{
Q_OBJECT
private:
//wnd data
int WIDTH = 1024;
int HEIGHT = 768;
//raw data
struct pernode* nbuf = 0;
int ncnt = 0;
struct perwire* wbuf = 0;
int wcnt = 0;
//qt data
QPushButton* btn = 0;
int bcnt = 0;
struct vert2d* v2d = 0;
int vcnt = 0;
struct vert2d* tmp = 0;
int tcnt = 0;

public:
MyMainWindow(QWidget* parent = 0) : QMainWindow(parent)
{
	qDebug() << "constructor";

	setWindowTitle("test");
	resize(WIDTH, HEIGHT);

	setAutoFillBackground(false);
//	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
}
virtual ~MyMainWindow()
{
	qDebug() << "destructor";
	freenodeandwire();
}

void prepnodeandwire(struct pernode* nb, int nc, struct perwire* wb, int wc)
{
	nbuf = nb;
	ncnt = nc;
	wbuf = wb;
	wcnt = wc;
	printf("ncnt=%d,wcnt=%d\n",ncnt,wcnt);

	int x,y,j;
	char str[64];
	srand(time(0));

	btn = (QPushButton*)malloc(ncnt * sizeof(QPushButton));
	v2d = (struct vert2d*)malloc(ncnt * sizeof(struct vert2d));
	tmp = (struct vert2d*)malloc(ncnt * sizeof(struct vert2d));

	for(j=0;j<ncnt;j++){
		if(_hash_ == nbuf[j].type){
			snprintf(str, 64, "%.4s\n%llx\n%.16s", (void*)&nbuf[j].type, nbuf[j].addr, nbuf[j].str);
		}
		else{
			snprintf(str, 64, "%.4s\n%llx", (void*)&nbuf[j].type, nbuf[j].addr);
		}

		new(&btn[j])QPushButton(str, this);
		connect(&btn[j], SIGNAL(clicked()), this, SLOT(onclick()));

		if(0 == j){
			x = WIDTH / 2;
			y = HEIGHT / 2;
		}
		else{
			x = rand() % WIDTH;
			y = rand() % HEIGHT;
		}

		v2d[j].x = x;
		v2d[j].y = y;
		btn[j].setGeometry(x-64,y-32, 128,64);
	}

	bcnt = vcnt = tcnt = ncnt;
}
void freenodeandwire()
{
	int j;
	for(j=0;j<bcnt;j++)btn[j].~QPushButton();
	free(btn);
	btn = 0;
	bcnt = 0;

	free(v2d);
	v2d = 0;
	vcnt = 0;

	free(tmp);
	tmp = 0;
	tcnt = 0;
}

protected:
void resizeEvent(QResizeEvent *event)
{
	qDebug() << "resize!" << width() << height();
	WIDTH = width();
	HEIGHT = height();
}
void paintEvent(QPaintEvent*)
{
	if(0 == nbuf)return;
	if(0 == ncnt)return;
	//
	if(0 == btn)return;
	if(0 == bcnt)return;
	if(0 == v2d)return;
	if(0 == vcnt)return;
	if(0 == tmp)return;
	if(0 == tcnt)return;

	forcedirected_2d(tmp,bcnt, v2d,bcnt, wbuf,wcnt);
	v2d[0].x = WIDTH/2;
	v2d[0].y = HEIGHT/2;

	int j,k;
	for(j=0;j<bcnt;j++){
		btn[j].setGeometry(v2d[j].x-64,v2d[j].y-32, 128,64);
	}

	//QColor c(Qt::gray);
	//c.setAlpha(100);
	//pt.fillRect(rect(), c);
	//Qpen pen;
	//pen.setColor(QColor(40, 115, 216)); pen.setWidth(2);
	//painter.setBrush(QBrush(Qt::red,Qt::SolidPattern));//设置画刷形式 
	//painter.drawLine(20,20,220,220);//画直线
	//painter.drawLine(20,220,220,20);
	//painter.drawEllipse(20,20,200,200);//画圆
	//painter.drawRect(20,20,200,200);//画矩形

	if(0 == wbuf)return;
	if(0 == wcnt)return;

	QPainter painter(this);
	painter.setPen(QPen(Qt::red, 1));//设置画笔形式

	int sx,sy,dx,dy;
	for(j=0;j<wcnt;j++){
		if(wbuf[j].src >= ncnt)continue;
		if(wbuf[j].dst >= ncnt)continue;
		//qDebug() << wbuf[j].src << "," << wbuf[j].dst;

		k = wbuf[j].src;
		sx = v2d[k].x;
		sy = v2d[k].y;

		k = wbuf[j].dst;
		dx = v2d[k].x;
		dy = v2d[k].y;

		painter.drawLine(sx,sy, dx,dy);
	}//foreach wire

	update();

}//func paintEvent


private slots:
void onclick()
{
	QPushButton* buttonSender = qobject_cast<QPushButton*>(sender()); // retrieve the button you have clicked
	QString buttonText = buttonSender->text();
	qDebug() << "click" << buttonText;
}
};//class mywindow




static int argc = 0;
static char* argv[1] = {0};
static QApplication* app = 0;;




extern "C"{


void render_data(struct pernode* nbuf, int ncnt, struct perwire* wbuf, int wcnt)
{
	qDebug() << "@render_data\n";

	MyMainWindow wnd;
	wnd.prepnodeandwire(nbuf,ncnt, wbuf,wcnt);
	wnd.show();
	qDebug() << "111111\n";

	app->exec();
	qDebug() << "222222\n";
}


void render_init(void* cb, int cl, void* lb, int ll)
{
	app = new QApplication(argc, argv);
	qDebug() << "@render_init\n";
}
void render_free()
{
	qDebug() << "@render_free\n";
	delete(app);
}


}//extern "C"




#include "qt.moc.cpp"
