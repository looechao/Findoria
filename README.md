C++ 网络编程 线程

## Roadmap

### Model1 Preparation

离线阶段准备的是在服务端启动之前就做好的事情，与用户发送请求之后再做大的事情不是同一个程序

- [x] 1. 语料整理

  - [x] 英文语料整理（梳理语料文件结构）
  - [x] 中文语料整理（梳理预料文件结构）

- [x] 2. 英文语料清洗（切割，空格）

  - [x] 读取所有语料的路径 

    src/makedict: inputFiles

  - [x] 存放raw数据

    src/DictProducer: setRaw

  - [x] 数据清洗（大写转小写，符号使用空格代替）

    src/DictProducer: cleanEnMaterial

  - [x] 词频统计

    src/DictProducer: buildEnDict

- [x] 3. 英文推荐词典生成（词频统计），将map 生成一个文件

  src/DictProducer: store

- [x] 4. 中文语料清洗（去除空格和标点符号）

  cleanCnMaterial

- [x] 5. 中文语料词典生成（分割+词典生成），自动合并进入dict.dat

- [x] 6. 生成并且存储索引文件

  - [x] 读取`vector<pair<string, int>>` 中的每一个单词的所有字符
  - [x] 存储到dictIndex.dat

### Model1 Online Part

- [x] 搭建服务器框架(WFREST[√]  or REACTOR )
- [x] 预热，将词典和词典索引加载到内存当中，读文件
- [x] 关键词分解，候选词召回
- [x] 返回响应

![](./assets/wordCandidate.gif)

### Model2 Preparation

- [ ] 数据清洗，生成网页库
- [ ] 网页去重，生成新网页库
- [ ] 网页偏移库
- [ ] 倒排索引库

### Model2 Online Part

- [ ] 预热，将数据加载到内存中
- [ ] 关键词全文搜索
- [ ] 相似度排序

## File structure

- `src/`：存放系统的源文件
- `include/`：存放系统的头文件
- `bin/`：存放系统的可执行程序
- `conf/myconf.conf`： 存放系统程序中所需的相关配置
- `data/dict.dat`：存放词典
- `data/dictindex.dat`：存放单词所在位置的索引库
- `data/newripepage.dat`：存放网页库
- `data/newoffset.dat`：存放网页的偏移库
- `data/invertIndex.dat`：存放倒排索引库
- `log/`：存放日志文件

## Problem

1. 单例模式冲突，即使创建了两个对象，使用的成员函数仍然是最先所创建的对象的

2. Http 请求中的中文是编码过的，需要解码后再处理

   ```c++
   string urlDecode(const std::string &encoded) {
       std::ostringstream decoded;
       decoded.fill('0');
       std::string::size_type len = encoded.length();
       for (std::string::size_type i = 0; i < len; ++i) {
           if (encoded[i] == '%') {
               if (i + 2 < len) {
                   std::string hex = encoded.substr(i + 1, 2);
                   int value = 0;
                   std::istringstream(hex) >> std::hex >> value;
                   decoded << static_cast<char>(value);
                   i += 2;
               }
           } else if (encoded[i] == '+') {
               decoded << ' ';
           } else {
               decoded << encoded[i];
           }
       }
       return decoded.str();
   }
   ```

3. 排序算法：候选词有`频率`和`最小编辑距离`两个主要的维度，需要选择合适的算法对候选词进行排序

   ![排序前](./assets/candidateProcess.png)

   按照最小编辑距离升序，若最小编辑距离相同，子按照热度的降序排列

   ![排序后](./assets/candidateProcessed.png)

