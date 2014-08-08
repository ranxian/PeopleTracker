using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace DataAnalyse
{
    public struct Vector3
    {
        public float x, y, z;
        public int r;

        public Vector3(float x = 0, float y = 0, float z = 0, int r = 0)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.r = r;
        }

        public float Dis
        {
            get
            {
                return (float)Math.Sqrt(x * x + y * y + z * z);
            }
        }

        public static Vector3 operator - (Vector3 v0, Vector3 v1)
        {
            return new Vector3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        }

        public static float operator * (Vector3 v0, Vector3 v1)
        {
            return v0.x * v1.x + v0.y * v1.y +  v0.z * v1.z;
        }

        public float angle(Vector3 v0, Vector3 v1)
        {
            Vector3 e0 = v0 - this, e1 = v1 - this;
            return (float)Math.Acos(e0*e1/(e0.Dis * e1.Dis));
        }
    }
    public class Frame
    {
        public const int N = 20;
        public List<Vector3> List = new List<Vector3>();
    }
    public class DataProcess
    {

        private static void GetSourceData(List<Frame> list)
        {
            FileStream stream = new FileStream("data.txt", FileMode.Open);
            StreamReader reader = new StreamReader(stream);
            while (!reader.EndOfStream)
            {
                Frame frame = new Frame();
                string str = reader.ReadLine();

                str = reader.ReadLine();
                while (str != "")
                {
                    int i = 0 ;
                    do
                    {
                        string[] splits = str.Split(' ');
                        Vector3 v = new Vector3();
                        v.x = float.Parse(splits[0]);
                        v.y = float.Parse(splits[1]);
                        v.z = float.Parse(splits[2]);
                        v.r = int.Parse(splits[3]);
                        frame.List.Add(v);
                        ++i;
                        str = reader.ReadLine();
                    } while (i < Frame.N);
                    
                }
                list.Add(frame);
            }
            reader.Close();
        }

        public static List<float> ExtractLeftElbowAngleFeature()
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            foreach (Frame frame in list)
            {
                if (frame.List.Count > 0)
                {
                    float angle = frame.List[5].angle(frame.List[4], frame.List[6]);
                    int reliabilty = frame.List[5].r + frame.List[4].r + frame.List[6].r;
                    // Console.WriteLine(angle);
                    if (angle >= 0f && angle <= 4 && reliabilty >= 6)
                        featureList.Add(angle);
                    else
                        featureList.Add(float.MaxValue);
                }
                else
                {
                    featureList.Add(float.MaxValue);
                }
            }
            return featureList;
        }

        public static List<float> ExtractRightElbowAngleFeature()
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            foreach (Frame frame in list)
            {
                if (frame.List.Count > 0)
                {
                    float angle = frame.List[9].angle(frame.List[8], frame.List[10]);
                    int reliabilty = frame.List[9].r + frame.List[8].r + frame.List[10].r;
                    // Console.WriteLine(angle);
                    
                    if (angle >= 0f && angle <= 4 && reliabilty >= 6)
                        featureList.Add(angle);
                    else
                        featureList.Add(float.MaxValue);
                }
                else
                {
                    featureList.Add(float.MaxValue);
                }
            }
            featureList = ListWithextremum.smooth(featureList);
            return featureList;
        }

        public static List<float> ExtractPositionFeature(Boolean ignoreZ = false)
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            for (int i=0;i<list.Count;++i)
            {
                float minV = float.MaxValue;
                if (list[i].List.Count > 0)
                {
                    for (int j = -10; j < 0; ++j)
                    {
                        if (i + j >= 0 && list[i + j].List.Count > 0 && list[i + j].List[0].r >= 2)
                        {
                            Vector3 disVec = list[i].List[0] - list[i + j].List[0];
                            if (ignoreZ)
                            {
                                disVec.z = 0;
                            }
                            minV = Math.Min(minV, disVec.Dis);
                        }
                    }
                }
                if (minV != float.MaxValue)
                {
                    minV = Math.Min(minV, 0.05f);
                }
                featureList.Add(minV);
            }
            featureList = ListWithextremum.smooth(featureList);
            return featureList;
        }

        public static List<float> ExtractHeadAngleFeature()
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            foreach (Frame frame in list)
            {
                if (frame.List.Count > 0)
                {
                    float angle = frame.List[2].angle(frame.List[1], frame.List[3]);
                    // Console.WriteLine(angle);
                    int reliability = frame.List[2].r + frame.List[1].r + frame.List[3].r;
                    if (angle >= 0f && angle <= 4 && reliability >= 0)
                        featureList.Add(angle);
                    else
                        featureList.Add(float.MaxValue);
                }
                else
                {
                    featureList.Add(float.MaxValue);
                }
            }
            return featureList;
        }
    }
}
