class Router {
    public:
        // socket_t socket;
        // UploadController upload_controller;
        // DownloadController download_controller;
        // DeleteController delete_controller;
        // ListController list_controller;
        // SynchronizeController synchronize_controller;
        void setup(); // setup the router
        void start(); // start handling the messages
};

void Router::setup() {
    // upload_controller = UploadController();
    // download_controller = DownloadController();
    // delete_controller = DeleteController();
    // list_controller = ListController();
    // synchronize_controller = SynchronizeController();
}

void Router::start() {
    /*
    while (true) {
        // receive message
        message = receive(socket);

        // handle message
        switch (message.type) {
            case MessageTypeEnum::UPLOAD:
                upload_controller.upload(socket, message);
                break;
            
            case MessageTypeEnum::DOWNLOAD:
                download_controller.download(socket, message);
                break;

            case MessageTypeEnum::DELETE:
                delete_controller.delete(socket, message);
                break;
            
            case MessageTypeEnum::LIST:
                list_controller.list(socket, message);
                break;
        }
    }
    */
}