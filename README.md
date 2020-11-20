Mediapipeを用いた顔(メッシュ状)及び手の3D座標の取得方法

以下では，[Mediapipe](https://github.com/google/mediapipe)を用いた顔と手と上半身の座標の取得方法
について説明します．
 
 ＊この置き換えにより，元のexamplesが動かなくなります．(どちらもできるよう現在対応中です)


映像からフレームごとに顔や手を認識して各点の座標をとり，CSVファイルとして保存していきます．

＊これはmacbook pro 16inch 2019 で行ったものです．Windows版では多少の違いがあるかもしれません．

＊また，これはリアルタイム映像ではなく，すでに記録した映像で行っています．

(追記@11/20)  
mediapipeの最新バージョンでは，hand trackingにおいて，multi hand tracking のフォルダがなくなりました．  
(これにより，handフォルダに両手のトラッキングが統合されたと思われます．)  
この変更により，handに関するファイルが正常に動作しなくなったので，現在修正中です．

### 1. [Mediapipe](https://github.com/google/mediapipe)を自分のパソコンにcloneし，環境構築を行う

環境構築の方法は，[公式のドキュメント](https://google.github.io/mediapipe/getting_started/getting_started.html)に記載されている方法に従って進めて下さい．

Mediapipeの使い方に関しては，[このサイト](https://note.com/npaka/n/n5008f82c43cc)などを活用してください．


### 2. 対象のファイルをcloneしたフォルダ内の以下に説明するディレクトリに追加or置き換えを行ってください．
(追加)

demo_run_graph_main_face.cc , demo_run_graph_main_hand.cc demo_run_graph_main_pose.cc→ /mediapipe/mediapipe/examples/desktop

(hand_landmark_3d.tflite → /mediapipe/mediapipe/models)  
(追記@9/25 このファイルは追加しなくても大丈夫です．)

(置き換え)

BUILD → /mediapipe/mediapipe/examples/desktop/BUILD

BUILD_hand → /mediapipe/mediapipe/examples/desktop/multi_hand_tracking/BUILD

BUILD_face → /mediapipe/mediapipe/examples/desktop/face_mesh/BUILD 

BUILD_pose → /mediapipe/mediapipe/examples/desktop/upper_body_pose_tracking/BUILD 



BUILD_handとBUILD_face，BUILD_poseに関しては，置き換えた後にBUILDという名前に戻してください

hand_landmark_cpu.pbtxt → /mediapipe/mediapipe/graphs/hand_tracking/subgraphs/  
(追記@9/28 このファイルは置換えないでください．)

### 3. 手の座標取得の場合は以下のコマンドを入力してください
```
$ bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/multi_hand_tracking:multi_hand_tracking_cpu

$ export GLOG_logtostderr=1
bazel-bin/mediapipe/examples/desktop/multi_hand_tracking/multi_hand_tracking_cpu --input_video_path=(入力するvideoのpath) --output_video_path=(出力するvideoのpath)
```
の順にターミナルに入力すると，/mediapipe/test_hand.csv　ができると思います．

### 4. 顔の座標取得の場合は以下のコマンドを入力してください
```
$ bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/face_mesh:face_mesh_cpu

$ GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/face_mesh/face_mesh_cpu --input_video_path=(入力するvideoのpath) --output_video_path=(出力するvideoのpath)
```
の順にターミナルに入力すると，/mediapipe/test_hand.csv　ができると思います．

### 4. 上半身の座標取得の場合は以下のコマンドを入力してください
```
$ bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/upper_body_pose_tracking:upper_body_pose_tracking_cpu

$ GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/upper_body_pose_tracking/upper_body_pose_tracking_cpu --input_video_path=(入力するvideoのpath) --output_video_path=(出力するvideoのpath)
```
の順にターミナルに入力すると，/mediapipe/test_pose.csv　ができると思います.  

#### (本来のリアルタイム&録画映像取得)(座標取得はできない)  

```

$ bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/face_mesh:face_mesh_cpu 

$ GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/face_mesh/face_mesh_cpu \
--calculator_graph_config_file=mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt  
(--input_video_path=... --output_video_path=... )


```
