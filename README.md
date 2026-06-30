# GAS Multiplayer Action RPG Project

## 📌 프로젝트 소개
- Unreal Engine GAS(Gameplay Ability System) 프레임워크 기반으로 구축한 멀티플레이어 액션 RPG 프로젝트입니다.
- GAS를 활용한 확장성 있는 전투 시스템 구현 및 서버-클라이언트 간의 안정적인 네트워크 동기화에 중점을 두었습니다.

## 🛠 사용 기술
- **Engine:** Unreal Engine 5.4.4 => 5.7.4
- **Language:** C++
- **Core System:** Gameplay Ability System (GAS)

## 💡 주요 기능 및 해결 과제
- **GAS 기반 전투 로직:** GameplayAbility와 AbilityTask등을 활용하여 Combo, Block, Counter등의 스킬 구현
- **네트워크 동기화:** Network Replication등을 활용한 데이터 동기화 및 서버 검증 로직.
- **AI 시스템:** Behavior Tree와 EQS를 이용한 몬스터 패턴 설계.

## 🎥 시연 영상
- [영상 링크](https://youtu.be/oB7ZN_HpaD8)

## 📂 핵심 코드 위치
- `Source/ActionGameProject/Private/AbilitySystem` : GAS 활용 주요 로직들 위치
- `Source/ActionGameProject/Private/AI` : AI 관련 로직들 위치(커스텀 비헤이비어 트리 노드 및 컨트롤러)
