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

        public Vector3(float x = 0, float y = 0, float z = 0)
        {
            this.x = x;
            this.y = y;
            this.z = z;
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
                    // Console.WriteLine(angle);
                    if (angle >= 0f && angle <= 4)
                        featureList.Add(angle);
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
                    // Console.WriteLine(angle);
                    if (angle >= 0f && angle <= 4)
                        featureList.Add(angle);
                }
                else
                {
                    featureList.Add(float.MaxValue);
                }
            }
            return featureList;
        }

        public static List<float> ExtractPositionFeature()
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            Frame lastFrame = null;
            foreach (Frame frame in list)
            {
                if (lastFrame != null && lastFrame.List.Count > 0 && frame.List.Count > 0)
                {
                    float feature = (frame.List[0] - lastFrame.List[0]).Dis;
                    feature = Math.Min(feature, 0.1f);
                    featureList.Add(feature);
                }
                else
                {
                    featureList.Add(float.MaxValue);
                }
                lastFrame = frame;
            }
            return featureList;
        }

        public static List<float> ExtractHeadAngleFeature()
        {
            List<Frame> list = new List<Frame>();
            GetSourceData(list);
            List<float> featureList = new List<float>();
            Frame lastFrame = null;
            foreach (Frame frame in list)
            {
                if (frame.List.Count > 0)
                {
                    float angle = frame.List[2].angle(frame.List[1], frame.List[3]);
                    // Console.WriteLine(angle);
                    if (angle >= 0f && angle <= 4)
                        featureList.Add(angle);
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
