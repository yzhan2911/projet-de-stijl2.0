/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tasks.h"
#include <stdexcept>

// Déclaration des priorités des taches
#define PRIORITY_TSERVER 30
#define PRIORITY_TOPENCOMROBOT 20
#define PRIORITY_TMOVE 20
#define PRIORITY_TSENDTOMON 22
#define PRIORITY_TRECEIVEFROMMON 25
#define PRIORITY_TSTARTROBOT 20
#define PRIORITY_TCAMERA 21
#define PRIORITY_TCHECKBATTERIE 20
#define PRIORITY_TWATCHDOG 22
#define PRIORITY_TRELOADMESSAGE 22
#define PRIORITY_OPENCAMERA 21
#define PRIORITY_CLOSECAMERA 21
#define PRIORITY_CAPTUREIMAGE 21
#define PRIORITY_CHERCHEAREA 20
#define PRIORITY_CHERCHEROBOT 21
/*
 * Some remarks:
 * 1- This program is mostly a template. It shows you how to create tasks, semaphore
 *   message queues, mutex ... and how to use them
 * 
 * 2- semDumber is, as name say, useless. Its goal is only to show you how to use semaphore
 * 
 * 3- Data flow is probably not optimal
 * 
 * 4- Take into account that ComRobot::Write will block your task when serial buffer is full,
 *   time for internal buffer to flush
 * 
 * 5- Same behavior existe for ComMonitor::Write !
 * 
 * 6- When you want to write something in terminal, use cout and terminate with endl and flush
 * 
 * 7- Good luck !
 */

/**
 * @brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void Tasks::Init() {
    int status;
    int err;
    
    /**************************************************************************************/
    /* 	Mutex creation                                                                    */
    /**************************************************************************************/
    if (err = rt_mutex_create(&mutex_monitor, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robot, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robotStarted, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_move, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
     if (err = rt_mutex_create(&mutex_checkbatterie, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_camera, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Mutexes created successfully" << endl << flush;

    /**************************************************************************************/
    /* 	Semaphors creation       							  */
    /**************************************************************************************/
    if (err = rt_sem_create(&sem_barrier, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_openComRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_serverOk, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_startRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_watchdog, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_reloadMessages, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
      if (err = rt_sem_create(&sem_openCamera, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
     if (err = rt_sem_create(&sem_closeCamera, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_captureImage, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_sem_create(&sem_chercheArea, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_sem_create(&sem_check_batterie, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_sem_create(&sem_chercheRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    
    
    cout << "Semaphores created successfully" << endl << flush;

    /**************************************************************************************/
    /* Tasks creation                                                                     */
    /**************************************************************************************/
    if (err = rt_task_create(&th_server, "th_server", 0, PRIORITY_TSERVER, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_sendToMon, "th_sendToMon", 0, PRIORITY_TSENDTOMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_receiveFromMon, "th_receiveFromMon", 0, PRIORITY_TRECEIVEFROMMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_openComRobot, "th_openComRobot", 0, PRIORITY_TOPENCOMROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_startRobot, "th_startRobot", 0, PRIORITY_TSTARTROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_move, "th_move", 0, PRIORITY_TMOVE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_checkBatterie, "th_checkBatterie", 0, PRIORITY_TCHECKBATTERIE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_watchdog, "th_watchdog", 0, PRIORITY_TWATCHDOG, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_reloadMessages, "th_RobotReloadMessage", 0, PRIORITY_TRELOADMESSAGE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_openCamera, "th_OpenCamera", 0, PRIORITY_OPENCAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_task_create(&th_closeCamera, "th_CloseCamera", 0, PRIORITY_CLOSECAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_task_create(&th_captureImage, "th_captureImage", 0, PRIORITY_CAPTUREIMAGE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_task_create(&th_chercheArea, "th_chercheArea", 0, PRIORITY_CHERCHEAREA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    } if (err = rt_task_create(&th_chercheRobot, "th_chercheRobot", 0, PRIORITY_CHERCHEROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Tasks created successfully" << endl << flush;

    /**************************************************************************************/
    /* Message queues creation                                                            */
    /**************************************************************************************/
    if ((err = rt_queue_create(&q_messageToMon, "q_messageToMon", sizeof (Message*)*50, Q_UNLIMITED, Q_FIFO)) < 0) {
        cerr << "Error msg queue create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Queues created successfully" << endl << flush;

}

/**
 * @brief Démarrage des tâches
 */
void Tasks::Run() {
    rt_task_set_priority(NULL, T_LOPRIO);
    int err;

    if (err = rt_task_start(&th_server, (void(*)(void*)) & Tasks::ServerTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_sendToMon, (void(*)(void*)) & Tasks::SendToMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_receiveFromMon, (void(*)(void*)) & Tasks::ReceiveFromMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_openComRobot, (void(*)(void*)) & Tasks::OpenComRobot, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_startRobot, (void(*)(void*)) & Tasks::StartRobotTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_move, (void(*)(void*)) & Tasks::MoveTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
     if (err = rt_task_start(&th_checkBatterie, (void(*)(void*)) & Tasks::CheckBattery, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
     if (err = rt_task_start(&th_watchdog, (void(*)(void*)) & Tasks::watchdog, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
        if (err = rt_task_start(&th_reloadMessages, (void(*)(void*)) & Tasks::RobotReloadMessage, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
            if (err = rt_task_start(&th_openCamera, (void(*)(void*)) & Tasks::OpenCamera, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_closeCamera, (void(*)(void*)) & Tasks::FermeCamera, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_captureImage, (void(*)(void*)) & Tasks::captureImage, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_task_start(&th_chercheArea, (void(*)(void*)) & Tasks::ChercheArea, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }if (err = rt_task_start(&th_chercheRobot, (void(*)(void*)) & Tasks::chercheRobot, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }

    cout << "Tasks launched" << endl << flush;
    
    camera=new Camera(sm,5);
}

/**
 * @brief Arrêt des tâches
 */
void Tasks::Stop() {
    monitor.Close();
    robot.Close();
    if(camera->IsOpen()){
    camera->Close();
    }
}

/**
 */
void Tasks::Join() {
    cout << "Tasks synchronized" << endl << flush;
    rt_sem_broadcast(&sem_barrier);
    pause();
}

/**
 * @brief Thread handling server communication with the monitor.
 */
void Tasks::ServerTask(void *arg) {
    int status;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are started)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task server starts here                                                        */
    /**************************************************************************************/
    rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
    status = monitor.Open(SERVER_PORT);
    rt_mutex_release(&mutex_monitor);

    cout << "Open server on port " << (SERVER_PORT) << " (" << status << ")" << endl;

    if (status < 0) throw std::runtime_error {
        "Unable to start server on port " + std::to_string(SERVER_PORT)
    };
    monitor.AcceptClient(); // Wait the monitor client
    cout << "Rock'n'Roll baby, client accepted!" << endl << flush;
    rt_sem_broadcast(&sem_serverOk);
}

/**
 * @brief Thread sending data to monitor.
 */
void Tasks::SendToMonTask(void* arg) {
    Message *msg;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task sendToMon starts here                                                     */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);

    while (1) {
        cout << "wait msg to send" << endl << flush;
        msg = ReadInQueue(&q_messageToMon);
        cout << "Send msg to mon: " << msg->ToString() << endl << flush;
        rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
        monitor.Write(msg); // The message is deleted with the Write
        rt_mutex_release(&mutex_monitor);
    }
}

/**
 * @brief Thread receiving data from monitor.
 */
void Tasks::ReceiveFromMonTask(void *arg) {
    Message *msgRcv;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task receiveFromMon starts here                                                */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);
    cout << "Received message from monitor activated" << endl << flush;

    while (1) {
        msgRcv = monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_MONITOR_LOST)) {
            delete(msgRcv);
            exit(-1);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) {
            rt_sem_v(&sem_openComRobot);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) {
            rt_sem_v(&sem_startRobot);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_GO_FORWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_BACKWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_LEFT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_RIGHT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_STOP)) {

            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            move = msgRcv->GetID();
            rt_mutex_release(&mutex_move);
        }else if(msgRcv->CompareID(MESSAGE_ROBOT_BATTERY_GET)){
     
            rt_sem_v(&sem_check_batterie);
       
        }else if(msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)){
             rt_sem_v(&sem_watchdog);
        }else if(msgRcv->CompareID(MESSAGE_CAM_OPEN)){
             rt_sem_v(&sem_openCamera);
        }else if(msgRcv->CompareID(MESSAGE_CAM_CLOSE)){
               rt_sem_v(&sem_closeCamera);
              rt_sem_p(&sem_captureImage ,TM_INFINITE);
        }else if(msgRcv->CompareID(MESSAGE_CAM_ASK_ARENA)){
             rt_sem_v(&sem_chercheArea);
        }else if(msgRcv->CompareID(MESSAGE_CAM_ARENA_CONFIRM)){
            savedArea=1;
            cout << "saved area=1" << endl << flush;
           
       
        }else if(msgRcv->CompareID(MESSAGE_CAM_ARENA_INFIRM)){
            savedArea=0;
            cout << "saved area=0" << endl << flush;
            
        }else if(msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_START)){
       
            rt_sem_v(&sem_chercheRobot);
        }else if(msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_STOP)){
            rt_sem_p(&sem_chercheRobot,TM_INFINITE);
        }
        delete(msgRcv); // mus be deleted manually, no consumer
    }
}

/**
 * @brief Thread opening communication with the robot.
 */
void Tasks::OpenComRobot(void *arg) {
    int status;
    int err;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task openComRobot starts here                                                  */
    /**************************************************************************************/
    while (1) {
        rt_sem_p(&sem_openComRobot, TM_INFINITE);
        cout << "Open serial com (";
        rt_mutex_acquire(&mutex_robot, TM_INFINITE);
        status = robot.Open();
        rt_mutex_release(&mutex_robot);
        cout << status;
        cout << ")" << endl << flush;

        Message * msgSend;
        if (status < 0) {
            msgSend = new Message(MESSAGE_ANSWER_NACK);
        } else {
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }
        WriteInQueue(&q_messageToMon, msgSend); // msgSend will be deleted by sendToMon
    }
}

/**
 * @brief Thread starting the communication with the robot.
 */
void Tasks::StartRobotTask(void *arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task startRobot starts here                                                    */
    /**************************************************************************************/
    while (1) {

        Message * msgSend;
        rt_sem_p(&sem_startRobot, TM_INFINITE);
        cout << "Start robot without watchdog (";
        rt_mutex_acquire(&mutex_robot, TM_INFINITE);
        msgSend = robot.Write(robot.StartWithoutWD());
        CompteurA3(msgSend);
        rt_mutex_release(&mutex_robot);
        cout << msgSend->GetID();
        cout << ")" << endl;

        cout << "Movement answer: " << msgSend->ToString() << endl << flush;
        WriteInQueue(&q_messageToMon, msgSend);  // msgSend will be deleted by sendToMon

        if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            robotStarted = 1;
            rt_mutex_release(&mutex_robotStarted);
        }
    }
}

/**
 * @brief Thread handling control of the robot.
 */
void Tasks::MoveTask(void *arg) {
    int rs;
    int cpMove;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    rt_task_set_periodic(NULL, TM_NOW, 100000000);

    while (1) {
        rt_task_wait_period(NULL);
        cout << "Periodic movement update";
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        if (rs == 1) {
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            cpMove = move;
            rt_mutex_release(&mutex_move);
            
            cout << " move: " << cpMove;
            
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            CompteurA3(robot.Write(new Message((MessageID)cpMove)));
            rt_mutex_release(&mutex_robot);
        }
        cout << endl << flush;
    }
}

/**
 * Write a message in a given queue
 * @param queue Queue identifier
 * @param msg Message to be stored
 */
void Tasks::WriteInQueue(RT_QUEUE *queue, Message *msg) {
    int err;
    if ((err = rt_queue_write(queue, (const void *) &msg, sizeof ((const void *) &msg), Q_NORMAL)) < 0) {
        cerr << "Write in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in write in queue"};
    }
}

/**
 * Read a message from a given queue, block if empty
 * @param queue Queue identifier
 * @return Message read
 */
Message *Tasks::ReadInQueue(RT_QUEUE *queue) {
    int err;
    Message *msg;

    if ((err = rt_queue_read(queue, &msg, sizeof ((void*) &msg), TM_INFINITE)) < 0) {
        cout << "Read in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in read in queue"};
    }/** else {
        cout << "@msg :" << msg << endl << flush;
    } /**/

    return msg;
}
void Tasks::CompteurA3(Message* msgRecever){
        if (msgRecever->GetID()==MESSAGE_ANSWER_ROBOT_TIMEOUT){
            compteur++;
             cout << "compteur+1"<<endl<<flush;
            if( compteur==3){
                cout << msgRecever->GetID()<<"compteur dépasse"<<endl<<flush;
                rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
                robotStarted = 0;
                rt_mutex_release(&mutex_robotStarted);
            }
        }else {
            compteur=0;
        }
   
    }

void Tasks::CheckBattery(void *arg){
    
    int rs;
    Message *level_batterie;
    cout << "Start Check batterie " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    rt_sem_p(&sem_check_batterie,TM_INFINITE);
    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    //Modifier période 500ms
    rt_task_set_periodic(NULL, TM_NOW, 500000000);
    
    while (1) {
        //Attendre Roobot commencer
        rt_task_wait_period(NULL);
        cout << "Batterie Check update";
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        if (rs == 1) {
            //roobot commence
             //write to robot, comrobot :: Write
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            level_batterie=robot.Write(robot.GetBattery());
            CompteurA3(level_batterie);
            rt_mutex_release(&mutex_robot);
            
            cout << " check batterie: " <<level_batterie->ToString()<<endl<<flush ;
             
            //envoyer message dans le queue, et moniteur va executer le tash dans le queue
            rt_mutex_acquire(&mutex_checkbatterie, TM_INFINITE);
            WriteInQueue(&q_messageToMon,level_batterie);
            rt_mutex_release(&mutex_checkbatterie);
        }
        cout << endl << flush;
    }
}



    void Tasks::watchdog(void *arg){
        
        cout << "Start watchdog " << __PRETTY_FUNCTION__ << endl << flush;
        
        rt_sem_p(&sem_barrier, TM_INFINITE);


            while (1) {

                Message * msgSend;
                rt_sem_p(&sem_watchdog,TM_INFINITE);
                cout << "Start robot with watchdog (";
                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                msgSend = robot.Write(robot.StartWithWD());
                CompteurA3(msgSend);
                rt_mutex_release(&mutex_robot);
                cout << msgSend->GetID();
                cout << ")" << endl;
                
                cout << "Movement answer: " << msgSend->ToString() << endl << flush;
                WriteInQueue(&q_messageToMon, msgSend);  
                if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
                    rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
                    robotStarted = 1;
                    rt_mutex_release(&mutex_robotStarted);
                }
                rt_sem_v(&sem_reloadMessages);
            }
       }
    
    void Tasks::RobotReloadMessage(void *arg){
 
        Message* status;

        cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;


        rt_sem_p(&sem_barrier, TM_INFINITE);
        rt_sem_p(&sem_reloadMessages, TM_INFINITE);

        cout << "Starting to monitor packet losses " << endl << flush;
        rt_task_set_periodic(NULL, TM_NOW, 1000*1000*1000);
        while(1) {
            rt_task_wait_period(NULL);

            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            int rs = robotStarted ;
            rt_mutex_release(&mutex_robotStarted);

            if (rs != 0) {
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            status = robot.Write(robot.ReloadWD());
            CompteurA3(status);
            rt_mutex_release(&mutex_robot);
            // send info to monitor
            if (status->GetID() < 0) {
                WriteInQueue(&q_messageToMon, new Message(MESSAGE_ANSWER_NACK));
                } 
            else {
                WriteInQueue(&q_messageToMon, new Message(MESSAGE_ANSWER_ACK));
             }
        }
        }    
    }
    void Tasks::OpenCamera(void *arg){
        
        rt_sem_p(&sem_barrier, TM_INFINITE);
         cout << "Open Camera"<<__PRETTY_FUNCTION__ << endl << flush;
        rt_sem_p(&sem_openCamera, TM_INFINITE);
      
        Message * msgSend;
        rt_mutex_acquire(&mutex_camera,TM_INFINITE);
        
        bool cameraStatus = camera->Open();
        rt_mutex_release(&mutex_camera);
        if(cameraStatus){
            msgSend=new  Message(MESSAGE_ANSWER_ACK);
        }else{
            msgSend=new Message(MESSAGE_ANSWER_NACK);
        }
        WriteInQueue(&q_messageToMon,msgSend);
        rt_sem_v(&sem_captureImage);
           
       
    }
    void Tasks::captureImage(void *arg){
        cout << "capture image " << __PRETTY_FUNCTION__ << endl << flush;
        rt_sem_p(&sem_barrier, TM_INFINITE);
        rt_sem_p(&sem_captureImage, TM_INFINITE);
        rt_sem_v(&sem_captureImage);
      
        cout << "Starting to capture image " << endl << flush;
        rt_task_set_periodic(NULL, TM_NOW, 100000000);
        while(1) {
            
            rt_task_wait_period(NULL);
             cout << "B1------------------------------- " << endl << flush;
            rt_sem_p(&sem_captureImage, TM_INFINITE);
            imag=(camera->Grab()).Copy();
            if (savedArea==1){
                imag->DrawArena(area);
            }
            rt_sem_v(&sem_captureImage);
             bool cameraStatus = camera->IsOpen();
            // send info to monitor
             cout << "B2------------------------------- " << endl << flush;
             rt_sem_p(&sem_captureImage, TM_INFINITE);
            if (cameraStatus>0) {
               WriteInQueue(&q_messageToMon,new MessageImg(MESSAGE_CAM_IMAGE,imag));
                } 
              rt_sem_v(&sem_captureImage);
            }
            
    
    }
    void Tasks::FermeCamera(void *arg){
        
        rt_sem_p(&sem_barrier, TM_INFINITE);
         rt_sem_p(&sem_closeCamera, TM_INFINITE);
        
            Message * msgSend;
           
            cout << "close Camera"<<__PRETTY_FUNCTION__ << endl << flush;
            rt_mutex_acquire(&mutex_camera,TM_INFINITE);
            camera->Close();
            bool cameraStatus = camera->IsOpen();
            rt_mutex_release(&mutex_camera);
            if(cameraStatus){
                msgSend=new  Message(MESSAGE_ANSWER_NACK);
            }else{
                msgSend=new Message(MESSAGE_ANSWER_ACK);
            }
            WriteInQueue(&q_messageToMon,msgSend);
            
    }
    
    void Tasks::ChercheArea(void * arg){
        
        rt_sem_p(&sem_barrier, TM_INFINITE);
             cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
        rt_sem_p(&sem_chercheArea,TM_INFINITE);
   
        Message * msgSend;
        Img* image;
        cout << "Commence à chercher AREA------------------------------------------------------------------------------" << endl << flush;
        rt_sem_p(&sem_captureImage, TM_INFINITE);
        cout << "j'ai bloqué le camera "<< endl << flush;
        image=(camera->Grab()).Copy();
        area=image->SearchArena();
        cout << "je cherche..." << endl << flush;
        if(area.IsEmpty()){
            msgSend=new Message(MESSAGE_ANSWER_NACK);
            cout<<"area vide-----------------------"<<endl <<flush;
            WriteInQueue(&q_messageToMon,msgSend);
        }else{
            cout << "j'ai trouvé----------------------" << endl << flush;
            image->DrawArena(area);
             WriteInQueue(&q_messageToMon,new MessageImg(MESSAGE_CAM_IMAGE,image));
        }
        rt_sem_v(&sem_captureImage); 
    
    }
    void Tasks::chercheRobot(void * arg) {
    rt_sem_p(&sem_barrier, TM_INFINITE);
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    rt_sem_p(&sem_chercheRobot, TM_INFINITE);
    rt_sem_v(&sem_chercheRobot);
    Position * positionRobot = nullptr; // Initialisez à nullptr pour éviter les pointeurs indéfinis
    Message * msgSend;
    rt_task_set_periodic(NULL, TM_NOW, 100000000);
    
    while(1) {
        rt_sem_p(&sem_chercheRobot, TM_INFINITE);
        rt_task_wait_period(NULL);   
        rt_sem_p(&sem_captureImage, TM_INFINITE);    

        if (area.IsEmpty()) {
            cout << "Veuillez définir 'area' au préalable." << endl << flush;
        } else {
             std::list<Position> positions = imag->SearchRobot(area);
            if (!positions.empty()) {
                positionRobot = &positions.front();
                cout << "Position du robot trouvée !!" << endl << flush;
                imag->DrawRobot(*positionRobot);
            }
        }
        rt_sem_v(&sem_captureImage);

        // Écriture dans la file d'attente
        if (positionRobot != nullptr) {
            WriteInQueue(&q_messageToMon, new MessagePosition(MESSAGE_CAM_POSITION, *positionRobot));
        }
         rt_sem_v(&sem_chercheRobot);
    }
}


