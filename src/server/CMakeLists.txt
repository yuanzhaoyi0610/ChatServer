#SRC_LIST包含了所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_lIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
#指定生成可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_lIST} ${MODEL_LIST} ${REDIS_LIST})
#依赖库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread)