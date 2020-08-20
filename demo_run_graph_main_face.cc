// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
#include <iostream>
#include <fstream>


#include <cstdlib>
#include <memory>
#include <vector>

#include "opencv2/opencv.hpp"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/port/commandlineflags.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

constexpr char kWindowName[] = "MediaPipe";
constexpr char kCalculatorGraphConfigFile[] =
    "mediapipe/graphs/face_mesh/face_mesh_desktop_live.pbtxt";
// Input and output streams.
constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";
constexpr char kMultiFaceLandmarksOutputStream[] = "multi_face_landmarks";

DEFINE_string(input_video_path, "",
              "Full path of video to load. "
              "If not provided, attempt to use a webcam.");
DEFINE_string(output_video_path, "",
              "Full path of where to save result (.mp4 only). "
              "If not provided, show result in a window.");

::mediapipe::Status RunMPPGraph(
    std::unique_ptr<::mediapipe::CalculatorGraph> graph) {

  LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const bool load_video = !FLAGS_input_video_path.empty();
  if (load_video) {
    capture.open(FLAGS_input_video_path);
  } else {
    LOG(INFO) << "Initialize the camera .";
    capture.open(1);
      if(capture.isOpened())//カメラデバイスが正常にオープンしたか確認．
         {
             //読み込みに成功したときの処理
              LOG(INFO) << "Can oopen";
         }
  }
  RET_CHECK(capture.isOpened());

  cv::VideoWriter writer;
  const bool save_video = !FLAGS_output_video_path.empty();
  if (!save_video) {
    cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    capture.set(cv::CAP_PROP_FPS, 30);
#endif
  }

  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller poller,
                   graph->AddOutputStreamPoller(kOutputStream));
  ASSIGN_OR_RETURN(::mediapipe::OutputStreamPoller multi_face_landmarks_poller,
                   graph->AddOutputStreamPoller(kMultiFaceLandmarksOutputStream));
  MP_RETURN_IF_ERROR(graph->StartRun({}));


    //csv出力
    using std::endl;
    using std::ofstream;
    ofstream ofs("test_face.csv");


  LOG(INFO) << "Start grabbing and processing frames.";
  bool grab_frames = true;
  while (grab_frames) {
    // Capture opencv camera or video frame.
    cv::Mat camera_frame_raw;
    
    capture >> camera_frame_raw;
      if (camera_frame_raw.empty()) {
           LOG(INFO) << "break.";
          break;}  // End of video.
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (!load_video) {
      cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }

    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<::mediapipe::ImageFrame>(
        ::mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        ::mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = ::mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Send image packet into the graph.
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph->AddPacketToInputStream(
        kInputStream, ::mediapipe::Adopt(input_frame.release())
                          .At(::mediapipe::Timestamp(frame_timestamp_us))));

    // Get the graph result packet, or stop if that fails.
    ::mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    auto& output_frame = packet.Get<::mediapipe::ImageFrame>();

    // Get the packet containing multi_face_landmarks.
    ::mediapipe::Packet multi_face_landmarks_packet;
    if (!multi_face_landmarks_poller.Next(&multi_face_landmarks_packet)) break;
    const auto& multi_face_landmarks =
        multi_face_landmarks_packet.Get<
            std::vector<::mediapipe::NormalizedLandmarkList>>();

    LOG(INFO) << "#Multi Face landmarks: " << multi_face_landmarks.size();
    int face_id = 0;
    for (const auto& single_face_landmarks: multi_face_landmarks) {
      ++face_id;
      LOG(INFO) << "Face [" << face_id << "]:";
      for (int i = 0; i < single_face_landmarks.landmark_size(); ++i) {
        const auto& landmark = single_face_landmarks.landmark(i);
        LOG(INFO) << "\tLandmark [" << i << "]: ("
                  << landmark.x() << ", "
                  << landmark.y() << ", "
                  << landmark.z() << ")";
           ofs << face_id << ", "<< i << ", "<< landmark.x() << ", "<< landmark.y() << ", "<< landmark.z() << ", "<< endl;

      }
    }

    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = ::mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    if (save_video) {
      if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(FLAGS_output_video_path,
                    ::mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
      }
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;
    }
  }

  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph->CloseInputStream(kInputStream));
  return graph->WaitUntilDone();
}

::mediapipe::Status InitializeAndRunMPPGraph() {

  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(::mediapipe::file::GetContents(
      kCalculatorGraphConfigFile, &calculator_graph_config_contents));
  LOG(INFO) << "Get calculator graph config contents: "
            << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  LOG(INFO) << "Initialize the calculator graph.";
  std::unique_ptr<::mediapipe::CalculatorGraph> graph =
      absl::make_unique<::mediapipe::CalculatorGraph>();
  MP_RETURN_IF_ERROR(graph->Initialize(config));

  return RunMPPGraph(std::move(graph));
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  ::mediapipe::Status run_status = InitializeAndRunMPPGraph();
  if (!run_status.ok()) {
    LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    return EXIT_FAILURE;
  } else {
    LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}
