Mediapipeを用いた顔(メッシュ状)及び手の3D座標の取得方法

以下では，[Mediapipe](https://github.com/google/mediapipe)を用いた顔と手の座標の取得方法
について説明します．


映像からフレームごとに顔や手を認識して各点の座標をとり，CSVファイルとして保存していきます．

### 1. [Mediapipe](https://github.com/google/mediapipe)を自分のパソコンにclone

Mediapipeの使い方に関しては，[このサイト](https://note.com/npaka/n/n5008f82c43cc)などを活用すると良いと思います．

### 2. 対象のファイルをcloneしたフォルダ内の以下に説明するディレクトリに追加or置き換えを行ってください．
(追加)

demo_run_graph_main_face.cc , demo_run_graph_main_hand.cc → /mediapipe/mediapipe/examples/desktop

hand_landmark_3d.tflite → /mediapipe/mediapipe/models

(置き換え)

BUILD → /mediapipe/mediapipe/examples/desktop/BUILD

BUILD_hand → /mediapipe/mediapipe/examples/desktop/multi_hand_tracking/BUILD

BUILD_face → /mediapipe/mediapipe/examples/desktop/face_mesh/BUILD

BUILD_handとBUILD_faceに関しては，置き換えた後にBUILDという名前にしてください

hand_landmark_cpu.pbtxt → /mediapipe/mediapipe/graphs/hand_tracking/subgraphs/


### 3. 手の座標取得の場合は以下のコマンドを入力してください

bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/multi_hand_tracking:multi_hand_tracking_cpu

export GLOG_logtostderr=1
bazel-bin/mediapipe/examples/desktop/multi_hand_tracking/multi_hand_tracking_cpu --input_video_path=(入力するvideoのpath)

の順にターミナルに入力すると，/mediapipe/test_hand.csv　ができると思います．

### 4. 顔の座標取得の場合は以下のコマンドを入力してください

bazel build -c opt --define MEDIAPIPE_DISABLE_GPU=1 mediapipe/examples/desktop/face_mesh:face_mesh_cpu

GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/face_mesh/face_mesh_cpu --input_video_path=(入力するvideoのpath)

の順にターミナルに入力すると，/mediapipe/test_face.csv　ができると思います．
