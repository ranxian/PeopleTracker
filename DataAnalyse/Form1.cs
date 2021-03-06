﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DataAnalyse
{
    public partial class Form1 : Form
    {
        private List<Button> buttonList = new List<Button>();
        public Form1()
        {
            InitializeComponent();
            buttonList.Add(button1);
            buttonList.Add(button2);
            buttonList.Add(button3);
            buttonList.Add(button4);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.myPanel1.SetList(0,(myPanel1.LList[0] == null)?DataProcess.ExtractRightElbowAngleFeature() : null);
        }

        protected override void OnSizeChanged(EventArgs e)
        {
            base.OnSizeChanged(e);
            this.myPanel1.Width = this.Width - 40;
            this.myPanel1.Height = this.Height - 100;
            int lastX = 0,lastY = this.ClientRectangle.Height - this.button1.Height - 20;
            foreach (var button in buttonList)
            {
                button.Location = new Point(lastX + 20, lastY);
                lastX += button.Width + 20;
            }
        }

        bool position_ignoreZ = false;
        private void button2_Click(object sender, EventArgs e)
        {
            if (myPanel1.LList[1] == null)
            {
                this.myPanel1.SetList(1, DataProcess.ExtractPositionFeature(false));
            }
            else
            {
                this.myPanel1.SetList(1, position_ignoreZ ? null : DataProcess.ExtractPositionFeature(true));
                position_ignoreZ = !position_ignoreZ;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            this.myPanel1.SetList(2, (myPanel1.LList[2] == null)?DataProcess.ExtractHeadAngleFeature() : null);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            this.myPanel1.SetList(3, (myPanel1.LList[3] == null) ? DataProcess.ExtractLeftElbowAngleFeature() : null);
        }
    }
}
