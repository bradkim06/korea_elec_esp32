# Filter Code

## **IIR 필터**

IIR 필터는 FIR 필터보다 훨씬 효율적이지만 선형 위상 왜곡을 유발할 수 있습니다.

- IIR 필터는 실시간 디지털 필터링을 위한 온라인 계산 채널로 설정될 수 있습니다.

- IIR 필터는 필터 응답 패턴을 정의하는 가변적인 Q 설정을 갖고 있습니다(하지만 FIR 필터에는 Q 구성 요소가 없습니다).

  - IIR 필터의 최적 Q는 0.707이며, 값이 낮을수록 응답이 평평해지고 값이 높을수록 응답이 더 뾰족해집니다. 정의에 따르면 Q가 0.707인 단일 차수 바이쿼드 필터는 버터워스 필터입니다.

- *Acq Knowledge*

  의 저역 통과 및 고역 통과 IIR 필터에 대한 기본 Q  는 0.707입니다. 대역 통과 및 대역 정지 필터에 대한 Q는 5입니다. 이러한 값은 거의 모든 필터 애플리케이션에 적합합니다.

### Example IIR Filter usage

```c
void filter_task(void* pvParameters) {
    // IIR 필터 적용
    apply_iir_filter(eeg_signal, filtered_signal, BLOCK_SIZE);

    // 결과 출력
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%f\n", filtered_signal[i]);
    }
}
```

## **FIR 필터**

FIR 필터는 IIR 필터보다 더 많은 컴퓨팅 파워가 필요합니다. 필터 응답은 IIR 필터보다 우수하도록 조정할 수 있습니다(즉, 특정 주파수를 감쇠하는 데 더 나은 작업을 하도록 조정할 수 있음).

FIR 필터의 계수 수 — 다음 경험 규칙은 FIR 필터를 설정할 때 사용할 올바른 계수 수를 결정하는 데 도움이 됩니다.
계수 수는 지정된 가장 낮은 차단 주파수로 나눈 샘플링 속도의 4배 이상이어야 합니다. 추가 계수는 응답을 개선합니다.

- 예를 들어, 256Hz로 샘플링된 데이터에 0.5Hz에서 저역 통과 필터를 실행하는 경우 필터에서 최소 (4 x 256/0.5) = 2,048개의 계수를 선택합니다.

### Example FIR Filter usage

```c
void filter_task(void* pvParameters) {
    // initialize once
    init_fir_filter();

    // IIR 필터 적용
    apply_fir_filter(eeg_signal, filtered_signal, BLOCK_SIZE);

    // 결과 출력
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%f\n", filtered_signal[i]);
    }
}
```
