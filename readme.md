# OpenGL 太空飛機專案

## 專案要求:

1. 製作3D前後Z軸滾動式的地圖 
2. 載入3DS太空船物件 (to load a 3DS object) 
3. 自訂隨機障礙物設計 (to load more than two 3DS objects randomly) 
4. 照相機平滑跟隨功能 (to control a camera to smoothly follow an object) 
5. 需執行太空船與障礙物之間的碰撞處理 (to add a sphere collider to an object, 
and to perform the collision detection) 
6. 以w a s d z x 等鍵盤移動控制 (鍵盤操控”前後左後上下”)，並使用cubic 
Bezier curve 配合太空船路徑移動 (to move an object with keyboards in terms 
of cubic Bezier curve) 
7. 遊戲時間倒數結束時，停留畫面並顯示得分 


## 執行方式:

執行 /Debug/tutorial4.exe


## 遊戲操作方式:
1.進入遊戲請先切換成英文輸入法

2.時間倒數40秒

3.wasd控制前後左右 zx控制上下

4.隨機生成小狗 隕石 兩種障礙物

5.障礙物有碰撞偵測  碰撞到計算分數 並且移除障礙物

6.分數計算規則

撞到小狗+1分
撞到隕石-1分

7.時間到顯示 小狗數量 隕石數量 分數=(小狗數量-隕石數量)

----------------------------------------------------------

特殊功能 :

1.q鍵切換wasd控制飛船xyz變化是使用世界座標 還是飛船local座標

2.當飛船移動  會以星球物件 顯示cubic Bezier curve接下來的4個移動點

3.照相機smooth follow 並且製作視角隨移動方向調整 從飛船移動方向後方 看向飛船移動方向前方

4.飛船製作旋轉功能 隨飛行方向rotatef

5.圓球形skybox

6.障礙物有隨機移動速度
