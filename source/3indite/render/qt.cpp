#include <ctime>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>
#include <QtWidgets/QPushButton>
#include <QtWebEngineWidgets/QWebEngineView>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define _hash_ hex32('h','a','s','h')




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
class MyMainWindow : public QMainWindow
{
public:
    MyMainWindow(QWidget* parent = 0) : QMainWindow(parent)
    {
	setWindowTitle("test");
	resize(1024,512);

        setAutoFillBackground(false);
//        setWindowFlags(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    struct perwire* wbuf = 0;
    int wcnt = 0;
    QPushButton** btn = 0;
    int btncnt = 0;
    void setWire(struct perwire* buf, int cnt, QPushButton** b)
    {
	wbuf = buf;
	wcnt = cnt;
	btn = b;
    }

protected:
    void paintEvent(QPaintEvent*)
    {
	if(0 == wbuf)return;
	if(0 == wcnt)return;
	if(0 == btn)return;

        //QPainter pt(this);
        //QColor c(Qt::gray);
        //c.setAlpha(100);
        //pt.fillRect(rect(), c);

	QPainter painter(this);
	painter.setPen(QPen(Qt::red, 1));//设置画笔形式
	//Qpen pen;
	//pen.setColor(QColor(40, 115, 216)); pen.setWidth(2);
	//painter.setBrush(QBrush(Qt::red,Qt::SolidPattern));//设置画刷形式 
	//painter.drawLine(20,20,220,220);//画直线
	//painter.drawLine(20,220,220,20);
	//painter.drawEllipse(20,20,200,200);//画圆
	//painter.drawRect(20,20,200,200);//画矩形

	int sx,sy,dx,dy,j,k;
	for(j=0;j<wcnt;j++){
		if(wbuf[j].src >= 16)continue;
		if(wbuf[j].dst >= 16)continue;
		//qDebug() << wbuf[j].src << "," << wbuf[j].dst;

		k = wbuf[j].src;
		sx = btn[k]->x() + btn[k]->width()/2;
		sy = btn[k]->y() + btn[k]->height()/2;

		k = wbuf[j].dst;
		dx = btn[k]->x() + btn[k]->width()/2;
		dy = btn[k]->y() + btn[k]->height()/2;

		painter.drawLine(sx,sy, dx,dy);
	}
    }
};




static int argc = 0;
static char* argv[1] = {0};
static QApplication* app = 0;;




extern "C"{


void render_node(struct pernode* nbuf, int ncnt, QPushButton* btn[], MyMainWindow* wnd)
{
	int x,y,j;
	char tmp[64];
	srand(time(0));

	if(ncnt>16)ncnt = 16;
	for(j=0;j<ncnt;j++){
		if(_hash_ == nbuf[j].type){
			snprintf(tmp, 64, "%.4s\n%llx\n%.16s", (void*)&nbuf[j].type, nbuf[j].addr, nbuf[j].str);
		}
		else{
			snprintf(tmp, 64, "%.4s\n%llx", (void*)&nbuf[j].type, nbuf[j].addr);
		}

		btn[j] = new QPushButton(tmp, wnd);
		if(0 == j){
			btn[j]->setGeometry(512-64,256-32, 128,64);
		}
		else{
			x = rand() % 1024;
			y = rand() % 512;
			btn[j]->setGeometry(x-64,y-32, 128,64);
		}
	}
}
void render_wire(struct perwire* wbuf, int wcnt, QPushButton* btn[], MyMainWindow* wnd)
{
/*	int x,y,j;
	for(j=0;j<wcnt;j++){
		if(wbuf[j].src >= 16)continue;
		if(wbuf[j].dst >= 16)continue;
		qDebug() << wbuf[j].src << "," << wbuf[j].dst;
	}*/
	wnd->setWire(wbuf, wcnt, btn);
}
void render_data(struct pernode* nbuf, int ncnt, struct perwire* wbuf, int wcnt)
{
	qDebug() << "@render_data\n";

	MyMainWindow wnd;

	QPushButton* btn[16] = {0};
	render_node(nbuf, ncnt, btn, &wnd);
	render_wire(wbuf, wcnt, btn, &wnd);

	wnd.show();

	app->exec();
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
