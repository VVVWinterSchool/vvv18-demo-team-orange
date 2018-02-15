// Authors: Francesca Palermo, Samuele Vinanzi

#include <string>

#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>
#include <opencv/cv.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;


/***************************************************/
class CtrlModule: public RFModule
{
protected:

    // LBP
    BufferedPort<Bottle> boxPort;

    //SFM
    Port rpcPort;

    //CONTROLLER
    BufferedPort<Bottle> controllerPort;

public:
    /***************************************************/
    bool configure(ResourceFinder &rf)
    {
        if (!boxPort.open("/orange/vision/box:i")){
            yError() << "Error while opening /orange/vision/box:i";
            return false;
        }
        else{
            yInfo() << "I correctly opened /orange/vision/box:i";
        }
        if (!rpcPort.open("/orange/vision/rpcclient")){
            yError() << "Error while opening /orange/vision/rpcclient";
            return false;
        }
        else{
            yInfo() << "I correctly opened /orange/vision/rpcclient";
        }
        if (!controllerPort.open("/orange/vision/controller:o")){
            yError() << "Error while opening /orange/vision/controller:o";
            return false;
        }
        else{
            yInfo() << "I correctly opened /orange/vision/controller:o";
        }
    }

    /***************************************************/
    bool interruptModule()
    {
        boxPort.interrupt();
        rpcPort.interrupt();
        controllerPort.interrupt();
        return true;
    }

    /***************************************************/
    bool close()
    {
        boxPort.close();
        rpcPort.close();
        controllerPort.close();
        return true;
    }

    /***************************************************/
    double getPeriod()
    {
        return 0.01;     // sync upon incoming images
    }

    /***************************************************/
    bool updateModule()
    {
        // Get data from the LBP module (bounding boxes and center of objects)
        yInfo() << "ei1";
        Bottle* boxBot = boxPort.read();
        yInfo() << "ei2";

        
        Matrix boxes(boxBot->size(), 4);    // Bounding boxes
        Matrix centers(boxBot->size(), 2);  // Center points of the bounding boxes
        for(int i=0; i<boxBot->size(); i++){
            Bottle* box = boxBot->get(i).asList();
            boxes[i][0] = box->get(0).asDouble();
            boxes[i][1] = box->get(1).asDouble();
            boxes[i][2] = box->get(2).asDouble();
            boxes[i][3] = box->get(3).asDouble();

            centers[i][0] = (boxes[i][0] + boxes[i][2]) / 2;
            centers[i][1] = (boxes[i][1] + boxes[i][3]) / 2;
        }

        // Get data from the SFM module
        Matrix worldCoords(centers.rows(), 3);
        Bottle cmd;
        Bottle reply;
        for(int i=0 ; i < centers.rows() ; i++){
            cmd.clear();
            reply.clear();
            cmd.addString("Root");
            yDebug() << "request x, y";
            yDebug() << centers[i][0];
            yDebug() << centers[i][1];
            yDebug() << "finish";

            cmd.addInt(centers[i][0]);
            cmd.addInt(centers[i][1]);
            rpcPort.write(cmd, reply);

            worldCoords[i][0] = reply.get(0).asDouble();
            worldCoords[i][1] = reply.get(1).asDouble();
            worldCoords[i][2] = reply.get(2).asDouble();
            yInfo() << worldCoords[i][0] << " " << worldCoords[i][1] << " " << worldCoords[i][2] << "\n";
        }
        
        //yInfo() << worldCoords[0][0] << " " << worldCoords[0][1] << " " << worldCoords[0][2] << "\n";

        // Pass back data to controller
        Bottle& output = controllerPort.prepare();
        output.clear();
        output.addList().read(boxes);
        output.addList().read(worldCoords);
        controllerPort.write();
        

        return true;
    }
};


/***************************************************/
int main(int argc, char *argv[])
{
    Network yarp;
    if (!yarp.checkNetwork())
    {
        yError()<<"YARP doesn't seem to be available";
        return 1;
    }

    ResourceFinder rf;
    rf.configure(argc,argv);

    CtrlModule mod;
    return mod.runModule(rf);
}
