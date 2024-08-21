# 伪目标：生成所有可执行文件
all: bin/Makedict bin/main bin/Makeweb

# 目标：生成 bin/Makedict
bin/Makedict: bin/Makedict.o bin/DictProducer.o bin/SplitTool.o
	g++ bin/Makedict.o bin/DictProducer.o bin/SplitTool.o -o bin/Makedict -g

# 目标：生成 bin/Makedict.o
bin/Makedict.o: src/Makedict.cc
	g++ -c src/Makedict.cc -o bin/Makedict.o -I include

# 目标：生成 bin/DictProducer.o
bin/DictProducer.o: src/DictProducer.cc
	g++ -c src/DictProducer.cc -o bin/DictProducer.o -I include

# 目标：生成 bin/SplitTool.o
bin/SplitTool.o: src/SplitTool.cc
	g++ -c src/SplitTool.cc -o bin/SplitTool.o -I include



# 目标: 生成 bin/main
bin/main: bin/main.o bin/SearchEngineServer.o bin/KeyRecomander.o
	g++ bin/main.o bin/SearchEngineServer.o bin/KeyRecomander.o -o bin/main -lwfrest -lworkflow -g

# 目标：生成 bin/main.o
bin/main.o: src/main.cc
	g++ -c src/main.cc -o bin/main.o -I include

# 目标：生成 bin/SearchEngineServer.o
bin/SearchEngineServer.o: src/SearchEngineServer.cc
	g++ -c src/SearchEngineServer.cc -o bin/SearchEngineServer.o -I include

# 目标：生成 bin/KeyRecomander.o
bin/KeyRecomander.o: src/KeyRecomander.cc
	g++ -c src/KeyRecomander.cc -o bin/KeyRecomander.o -I include



# 目标：生成 bin/Makeweb
bin/Makeweb: bin/Makeweb.o bin/PagelibProcessor.o bin/tinyxml2.o
	g++ bin/Makeweb.o bin/PagelibProcessor.o bin/tinyxml2.o -o bin/Makeweb -g

# 目标：生成 bin/Makeweb.o
bin/Makeweb.o: src/Makeweb.cc
	g++ -c src/Makeweb.cc -o bin/Makeweb.o -I include

# 目标：生成 bin/PagelibProcessor.o
bin/PagelibProcessor.o: src/PagelibProcessor.cc
	g++ -c src/PagelibProcessor.cc -o bin/PagelibProcessor.o -I include

# 目标：生成 bin/tinyxml2.o
bin/tinyxml2.o: src/tinyxml2.cpp
	g++ -c src/tinyxml2.cpp -o bin/tinyxml2.o -I include

# 清理命令
clean:
	rm -f bin/*