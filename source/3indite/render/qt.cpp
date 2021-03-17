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
#define WIDTH 1024
#define HEIGHT 768




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
public:
	MyMainWindow(QWidget* parent = 0) : QMainWindow(parent)
	{
		setWindowTitle("test");
		resize(WIDTH, HEIGHT);

		setAutoFillBackground(false);
//		setWindowFlags(Qt::FramelessWindowHint);
		setAttribute(Qt::WA_TranslucentBackground, true);
	}

	QPushButton** btn = 0;
	int bcnt = 0;
	struct vert2d* v2d = 0;
	int vcnt = 0;
	struct vert2d* tmp = 0;
	int tcnt = 0;
	void setNode(QPushButton** b, int bl, struct vert2d* vb, int vl, struct vert2d* tb, int tl)
	{
		btn = b;
		bcnt = bl;
		v2d = vb;
		vcnt = vl;
		tmp = tb;
		tcnt = tl;
	}

	struct perwire* wbuf = 0;
	int wcnt = 0;
	void setWire(struct perwire* buf, int cnt)
	{
		wbuf = buf;
		wcnt = cnt;
	}

protected:
	void paintEvent(QPaintEvent*)
	{
		if(0 == wbuf)return;
		if(0 == wcnt)return;
		if(0 == btn)return;
		if(0 == bcnt)return;

		forcedirected_2d(tmp,bcnt, v2d,bcnt, wbuf,wcnt);
		v2d[0].x = WIDTH/2;
		v2d[0].y = HEIGHT/2;

		int j,k;
		for(j=0;j<bcnt;j++){
			btn[j]->setGeometry(v2d[j].x-64,v2d[j].y-32, 128,64);
		}

		//QPainter pt(this);
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

		QPainter painter(this);
		painter.setPen(QPen(Qt::red, 1));//设置画笔形式

		int sx,sy,dx,dy;
		for(j=0;j<wcnt;j++){
			if(wbuf[j].src >= 16)continue;
			if(wbuf[j].dst >= 16)continue;
			//qDebug() << wbuf[j].src << "," << wbuf[j].dst;

			k = wbuf[j].src;
			//sx = btn[k]->x() + btn[k]->width()/2;
			//sy = btn[k]->y() + btn[k]->height()/2;
			sx = v2d[k].x;
			sy = v2d[k].y;

			k = wbuf[j].dst;
			//dx = btn[k]->x() + btn[k]->width()/2;
			//dy = btn[k]->y() + btn[k]->height()/2;
			dx = v2d[k].x;
			dy = v2d[k].y;

			painter.drawLine(sx,sy, dx,dy);
		}//foreach wire
	}//func paintEvent
};//class mywindow




static int argc = 0;
static char* argv[1] = {0};
static QApplication* app = 0;;




extern "C"{


void render_node(struct pernode* nbuf, int ncnt, QPushButton* btn[], MyMainWindow* wnd,
	struct vert2d* vbuf,int vlen, struct vert2d* tbuf, int tlen)
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
			x = WIDTH/2;
			y = HEIGHT/2;
		}
		else{
			x = rand() % WIDTH;
			y = rand() % HEIGHT;
		}

		vbuf[j].x = x;
		vbuf[j].y = y;
		btn[j]->setGeometry(x-64,y-32, 128,64);
	}

	wnd->setNode(btn, ncnt, vbuf,ncnt, tbuf,ncnt);
}
void render_data(struct pernode* nbuf, int ncnt, struct perwire* wbuf, int wcnt)
{
	qDebug() << "@render_data\n";

	MyMainWindow wnd;

	struct vert2d v2d[16];
	struct vert2d tmp[16];
	QPushButton* btn[16] = {0};
	render_node(nbuf, ncnt, btn, &wnd, v2d, 16, tmp, 16);
	wnd.setWire(wbuf, wcnt);

	wnd.show();

	app->exec();

	int j;
	for(j=0;j<16;j++){
		if(btn[j])delete btn[j];
	}
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
