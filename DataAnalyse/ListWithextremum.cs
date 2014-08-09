using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DataAnalyse
{
    public class ListWithextremum : List<float>
    {
        private float vMin, vMax;
        public float VMax
        {
            get
            {
                return vMax;
            }
        }

        public float VMin
        {
            get
            {
                return vMin;
            }
        }
        public ListWithextremum(IList<float> list)
            : base(list)   
        {
            vMin = float.MaxValue;
            vMax = float.MinValue; 
            if (list.Count > 0)
            {
                foreach (float t in list)
                {
                    if (t != float.MaxValue)
                    {
                        if (t.CompareTo(vMax) > 0)
                        {
                            vMax = t;
                        }

                        if (t.CompareTo(vMin) < 0)
                        {
                            vMin = t;
                        }
                    }
                }
            }
        }

        public static int smoothFrames = 9;

        public static List<float> smooth(IList<float> list)
        {
            int left = smoothFrames / 2, right = (smoothFrames + 1) / 2;
            List<float> ret = new List<float>();
            for (int i=0;i<list.Count;++i)
            {
                float v = 0;
                int count = 0;
                for (int j=i-left;j<i+right;++j)
                {
                    if (j >=0 && j<list.Count && list[j]!=float.MaxValue)
                    {
                        v += list[j];
                        count++;
                    }
                }
                if (count == 0)
                {
                    ret.Add(float.MaxValue);
                }
                else
                {
                    ret.Add(v / count);
                }
                
            }
            return ret;
        }
    
    }
}
