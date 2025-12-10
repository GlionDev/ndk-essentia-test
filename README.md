## Attempt to port Python Librosa to Android NDK (C++) (using Essentia)

### 목적
Python `Librosa` 라이브러리를 통해 학습된 AI 모델의 입력 데이터를 Android 디바이스 내에서 생성(전처리)하기 위함. 
즉, **Librosa의 특징 추출(Feature Extraction)** 로직을 **Android NDK(C++) 환경으로 포팅**하여 On-device 추론 파이프라인을 구축하는 것을 목표로 함.

### 결과
**Essentia 를 사용한 포팅 시도는 중단 결정.**
학습된 모델은 Chromagram 특징 추출 시 CQT 방식을 요구하지만, Essentia 의 알고리즘적 특성상 정합성(유사도 0.97) 확보가 불가능함을 확인.

### Essentia 도입 중단 사유
1. 알고리즘의 태생적 불일치
    - **Librosa** : 모델 학습에 사용된 데이터는 Librosa 기반으로 추출한 특징 데이터로 학습되었음. Chromagram 추출 시 사용된 
    `librosa.feature.chroma_cqt` 는 재귀적 다운샘플링(Recursive Downsampling) 방식을 사용하여 시간 영역에서 파형을 정교하게 분석하므로 특히 저음역대의
    해상도가 매우 뛰어남.
    - **Essentia** : Chromagram 알고리즘에 사용되는 `ConstantQ` 알고리즘은 FFT 기반의 근사 방식을 사용함.
      - 작동 원리 : 전체 프레임에 대해 FFT 를 1회 수행한 뒤, 주파수 영역에서 CQT 커널을 곱해 값을 추정함
      - 문제점 : 연산속도는 빠르지만, FFT 에 고정된 윈도우 크기로 인해 Librosa 특유의 가변적 시간 해상도를 물리적으로 따라갈 수 없음
    - **결론** : Librosa 와의 차이점을 따라잡고자 별도의 C++ 로직 추가 시, **오히려 연산 속도의 저하로 이어짐**
2. 정합성 테스트 결과
    - **유사도 미달** : 두 라이브러리 간 추출된 Chromagram의 코사인 유사도(Cosine Similarity) 측정 결과, 평균 0.88 수준에 그침. (목표치: 0.97 이상)
    - **성능 이슈** : Essentia의 FFT 방식은 저음역대가 뭉개지거나(Blurring) 시간 축 정렬(Alignment)이 미세하게 어긋나는 현상이 발생함. 
    이를 보정하기 위해 로직을 수정할 경우 처리 시간이 2~3초까지 늘어나, NDK를 사용하는 이점(실시간성/고속처리)이 사라짐.

### 참고
- 학습된 모델과 샘플 음원은 제외
