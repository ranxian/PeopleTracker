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
    
    }
}
