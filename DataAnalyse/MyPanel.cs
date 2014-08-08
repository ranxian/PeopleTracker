using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

namespace DataAnalyse
{
    public partial class MyPanel : UserControl
    {
        public MyPanel()
        {
            InitializeComponent();
            List<float> tmpList = new List<float>();
            for (int i = 0; i < 120; ++i)
            {
                tmpList.Add(5f);
            }

            this.DoubleBuffered = true;
        }

        private bool reDrawData = true;
        private Bitmap dataBitmap;

        private int mouseX, mouseY;

        private ListWithextremum[] lList = new ListWithextremum[10];

        private static Color[] colors = new Color[]
        {
            Color.Red,
            Color.Green,
            Color.Blue,
            Color.Purple,
            Color.Purple,
            Color.Purple,
            Color.Purple,
            Color.Purple,
            Color.Purple,
            Color.Purple,
        };

        public ListWithextremum[] LList
        {
            get
            {
                return lList;
            }
        }

        public void SetList(int i, IList<float> list)
        {
            if (list == null)
            {
                lList[i] = null;
            }
            else
            {
                lList[i] = new ListWithextremum(list);
            }
            reDrawData = true;
            Invalidate();
        }


        private const int bottomMargin = 40;

        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            dataBitmap = new Bitmap(Math.Max(1,this.Width), Math.Max(1,this.Height));
            //Console.WriteLine(this.Width + " " + this.Height);
            reDrawData = true;
            Invalidate();
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);
            mouseX = e.X;
            mouseY = e.Y;
            Invalidate();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            int w = this.Width, h = this.Height - bottomMargin;
            Graphics g;
            Font font = new Font("宋体", 15, FontStyle.Bold);

            int textLength;

            if (reDrawData)
            {
                g = Graphics.FromImage(dataBitmap);
                g.Clear(this.BackColor);
                g.FillRectangle(new SolidBrush(Color.White), 0, 0, w, h);

                textLength = 20;

                for (int listNumber = 0; listNumber < lList.Length; ++listNumber)
                {
                    Color color = colors[listNumber];
                    List<Point> pList = new List<Point>();
                    IList<float> list = lList[listNumber];
                    if (list == null) continue;
                    float vMax, vMin;
                    vMax = lList[listNumber].VMax;
                    vMin = lList[listNumber].VMin;
                    for (int i = 0; i < list.Count; ++i)
                    {
                        float v = list[i];
                        int x = (int)((float)i / list.Count * w);
                        int y;
                        if (v != float.MaxValue)
                        {

                            if (vMax == vMin)
                            {
                                y = h / 2;
                            }
                            else
                            {
                                y = h - (int)(h * (v - vMin) / (vMax - vMin));
                            }
                            pList.Add(new Point(x, y));
                            //g.DrawRectangle(pen, x, y, 1, 1);
                        }
                        else
                        {
                            y = 0;
                            g.FillRectangle(new SolidBrush(Color.Blue), x, y, w * (i + 1) / list.Count - x, h);
                        }
                    }
                    GraphicsPath path = new GraphicsPath();
                    for (int i = 0; i < pList.Count - 1; ++i)
                    {
                        path.AddLine(pList[i].X, pList[i].Y, pList[i + 1].X, pList[i + 1].Y);
                    }
                    g.DrawPath(new Pen(color), path);
                    String text = "vMin=" + vMin + ",vMax=" + vMax;
                    g.DrawString(text, font, new SolidBrush(color), new Point(textLength, h));
                    textLength += (int)(g.MeasureString(text, font).Width + 20);
                }
                reDrawData = false;
            }
            g = e.Graphics;
            g.DrawImage(dataBitmap, new Point(0, 0));

            textLength = 20;
            for (int listNumber = 0; listNumber < lList.Length; ++listNumber)
            {
                Color color = colors[listNumber];
                IList<float> list = lList[listNumber];
                if (list == null) continue;
                float vMax, vMin;
                vMax = lList[listNumber].VMax;
                vMin = lList[listNumber].VMin;

                int fn = list.Count * mouseX / w;
                if (fn < list.Count)
                {
                    string text = "fn=" + fn + ";v=" + list[fn] + ";" + (float)fn / 30;
                    g.DrawString(text, font, new SolidBrush(color), new Point(textLength, h + bottomMargin / 2));
                    textLength += (int)(g.MeasureString(text, font).Width + 20);

                }
            }
        }
    }
}
