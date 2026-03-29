// Firebase 초기화 및 데이터베이스 설정
console.log('storyboard.js loaded'); // 파일 로드 확인

const firebaseConfig = {
    databaseURL: "https://true-s-default-rtdb.firebaseio.com"
};

const app = firebase.initializeApp(firebaseConfig);
const db = firebase.database();

// 전역 상태 변수: 스토리보드 데이터, 연대 목록, 현재 선택된 ID와 탭
window.state = {
    storyboard: [],  // 타임라인 이벤트 목록
    eras: [],  // 연대 목록 (DB에서 불러옴, 없으면 기본값 사용)
    currentId: null,  // 현재 선택된 이벤트 ID
    currentTab: 'timeline'  // 현재 활성 탭
};

// 임시 문서 저장소 (스토리 관련)
let documents = [
    {
        id: 'doc1',
        title: '단락 1',
        text: '첫 번째 단락: 주인공이 모험을 시작한다.'
    }
];

// Firebase에서 데이터를 동기화하는 함수
function syncData() {
    // 연대 설정 동기화 (없으면 기본값 설정)
    db.ref('users/Yuta/projects/기본/settings/eras').on('value', snap => {
        const erasData = snap.val();
        if (erasData && erasData.length > 0) {
            window.state.eras = erasData;
        } else {
            // DB에 기본값이 없으면 설정하고 사용
            window.state.eras = ['제국력', '대한민국'];
            db.ref('users/Yuta/projects/기본/settings/eras').set(window.state.eras);
        }
        updateEraSelect();
        renderEras();
    });

    // 스토리보드 데이터 동기화
    db.ref('users/Yuta/projects/기본/storyboard').on('value', snap => {
        const raw = snap.val();
        if (!raw) {
            window.state.storyboard = [];
            renderTimeline();
            return;
        }
        // 데이터를 정렬하여 상태에 저장
        window.state.storyboard = Object.keys(raw)
            .map(key => ({ ...raw[key], id: String(key) }))
            .sort((a, b) => (a.order || 0) - (b.order || 0));
        renderTimeline();
    });
}

// 탭 전환 함수: 탭을 클릭하면 해당 뷰를 활성화
function showTab(n, el) {
    console.log('showTab called with:', n, el); // 디버깅용 로그
    try {
        window.state.currentTab = n;  // 현재 탭 상태 업데이트
        // 모든 탭과 뷰에서 active 클래스 제거
        document.querySelectorAll('.tab-item').forEach(t => t.classList.remove('active'));
        document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
        // 선택된 탭과 뷰에 active 클래스 추가
        el.classList.add('active');
        document.getElementById(n + '-view').classList.add('active');
        closePopMenus();  // 팝업 메뉴 닫기
        console.log('showTab completed successfully');
    } catch (error) {
        console.error('Error in showTab:', error);
    }
}

// 플로팅 액션 버튼(FAB) 클릭 핸들러: 탭에 따라 다른 동작 수행
function handleFabClick() {
    closePopMenus();  // 팝업 메뉴 닫기
    if (window.state.currentTab === 'timeline') {
        openAddModal();  // 타임라인 탭: 새 이벤트 추가 모달 열기
    } else if (window.state.currentTab === 'story') {
        document.getElementById('story-menu').classList.toggle('active');  // 스토리 탭: 메뉴 토글
    } else if (window.state.currentTab === 'char') {
        document.getElementById('char-menu').classList.toggle('active');  // 캐릭터 탭: 메뉴 토글
    }
}

// 모든 팝업 메뉴 닫기
function closePopMenus() {
    document.querySelectorAll('.pop-menu').forEach(m => m.classList.remove('active'));
}

// 스토리 드로어 열기: 문서 목록 렌더링 후 드로어 표시
function openDrawer() {
    renderDocumentList();
    document.getElementById('story-drawer').classList.add('active');
    document.querySelector('.drawer-backdrop').classList.add('active');
}

// 스토리 드로어 닫기
function closeDrawer() {
    document.getElementById('story-drawer').classList.remove('active');
    document.querySelector('.drawer-backdrop').classList.remove('active');
}

// 문서 목록 렌더링: 임시 문서 목록을 HTML로 생성
function renderDocumentList() {
    const list = document.getElementById('treatment-list');
    list.innerHTML = '';  // 기존 목록 초기화
    documents.forEach(doc => {
        const item = document.createElement('div');
        item.style.cssText = `
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            border-bottom: 1px solid #eee;
            background: #fff;
            border-radius: 6px;
            margin-bottom: 5px;
        `;
        item.innerHTML = `
            <div style="font-size: 13px; cursor: pointer;" onclick="loadDocument('${doc.id}')">📄 ${doc.title}</div>
            <div style="cursor: pointer; font-weight: bold; color: #adb5bd; padding: 0 5px;" onclick="editDocMenu('${doc.id}')">=</div>
        `;
        list.appendChild(item);
    });
}

// 문서 로드: 선택된 문서를 텍스트 영역에 표시
function loadDocument(id) {
    const doc = documents.find(d => d.id === id);
    if (doc) {
        document.getElementById('story-treatment').value = doc.text;
        closeDrawer();
    }
}

// 문서 편집 메뉴: 수정 또는 삭제 선택
function editDocMenu(id) {
    const action = prompt("원하는 동작을 입력하세요 (수정 / 삭제)");
    if (action === "수정") {
        const newTitle = prompt("새로운 제목을 입력하세요");
        if (newTitle) {
            documents.find(d => d.id === id).title = newTitle;
            renderDocumentList();
        }
    } else if (action === "삭제") {
        documents = documents.filter(d => d.id !== id);
        renderDocumentList();
    }
}

// 폴더 생성 (준비 중)
function createFolder() {
    alert('폴더 생성 기능 준비중');
}

// 단락 내보내기: 현재 텍스트를 새 문서로 저장
function exportParagraph() {
    const text = document.getElementById('story-treatment').value;
    const newDoc = {
        id: 'doc' + Date.now(),
        title: '단락 ' + (documents.length + 1),
        text: text
    };
    documents.push(newDoc);
    alert(`[${newDoc.title}] 문서로 내보내졌습니다! 목록에서 확인하세요.`);
}

// 타임라인 렌더링: 스토리보드 데이터를 연도/월별로 그룹화하여 HTML 생성
function renderTimeline() {
    const list = document.getElementById('tl-list');
    list.innerHTML = '';  // 기존 타임라인 초기화

    // 데이터를 제국력별로 그룹화
    const grouped = window.state.storyboard.reduce((acc, cur) => {
        if (!acc[cur.era]) acc[cur.era] = {};
        if (!acc[cur.era][cur.year]) acc[cur.era][cur.year] = {};
        if (!acc[cur.era][cur.year][cur.month]) acc[cur.era][cur.year][cur.month] = [];
        acc[cur.era][cur.year][cur.month].push(cur);
        return acc;
    }, {});

    // 각 제국력별로 그룹 생성
    for (const era in grouped) {
        const eraGroup = document.createElement('div');
        eraGroup.className = 'era-group';
        eraGroup.dataset.era = era;

        // 제국력 배너
        const eraBanner = document.createElement('div');
        eraBanner.className = 'era-banner';
        eraBanner.innerHTML = `<span class="era-text">${era}</span>`;
        eraGroup.appendChild(eraBanner);

        // 각 연도별로 배너와 클러스터 생성
        for (const year in grouped[era]) {
            const yearBanner = document.createElement('div');
            yearBanner.className = 'year-banner';
            yearBanner.innerHTML = `<span class="year-text">${era} ${year}년</span>`;
            eraGroup.appendChild(yearBanner);

            for (const month in grouped[era][year]) {
                if (grouped[era][year][month].length === 0) continue;

                const cluster = document.createElement('div');
                cluster.className = 'month-cluster';
                cluster.innerHTML = `<div class="month-node"></div>`;

                const grid = document.createElement('div');
                grid.className = 'event-grid';
                grid.dataset.month = month;
                grid.dataset.yearKey = `${era} ${year}년`;

                // 각 이벤트 카드 생성
                grouped[era][year][month].forEach((ev) => {
                    const card = document.createElement('div');
                    card.className = 'timeline-card';
                    card.dataset.id = String(ev.id);
                    card.innerHTML = `
                        <div class="card-index"></div>
                        <div onclick="openDetailModal('${ev.id}')">
                            <div style="font-size: 10px; color: var(--node); font-weight: bold;">${month}월</div>
                            <div style="font-size: 12px; margin-top: 5px; line-height: 1.5;">${ev.text}</div>
                        </div>
                    `;
                    grid.appendChild(card);
                });

                cluster.appendChild(grid);
                eraGroup.appendChild(cluster);

                // 드래그 앤 드롭 기능 추가 (이벤트 카드용)
                console.log('Creating Sortable for grid:', grid, 'with items:', grid.children.length);
                if (typeof Sortable !== 'undefined') {
                    const sortable = new Sortable(grid, {
                        group: 'events', // 이벤트 카드 그룹
                        animation: 200,
                        delayOnTouchStart: 3,
                        touchStartThreshold: 3,
                        forceFallback: false,
                        removeClone: true,
                        swapThreshold: 0.5,
                        ghostClass: 'sortable-ghost',
                        chosenClass: 'sortable-chosen',
                        dragClass: 'sortable-drag',
                        onStart: function(evt) {
                            console.log('Event drag started:', evt.item.dataset.id);
                        },
                        onEnd: function(evt) {
                            console.log('Event drag ended:', evt.item.dataset.id, 'to', evt.to.dataset.month);
                            const movedId = evt.item.dataset.id;
                            const newMonth = evt.to.dataset.month;
                            const parts = evt.to.dataset.yearKey.split(' ');

                            // Firebase에 업데이트
                            if (db && db.ref) {
                                db.ref(`users/Yuta/projects/기본/storyboard/${movedId}`).update({
                                    month: newMonth,
                                    era: parts[0],
                                    year: parts[1].replace('년', '')
                                }).then(() => {
                                    console.log('Firebase update successful');
                                    reorderAll();
                                }).catch(error => {
                                    console.error('Firebase update failed:', error);
                                });
                            } else {
                                console.error('Database not available');
                            }
                        }
                    });
                    console.log('Event Sortable created:', sortable);
                } else {
                    console.error('Sortable library not loaded');
                }
            }
        }

        list.appendChild(eraGroup);
    }

    // 제국력 그룹 드래그 앤 드롭 기능 추가
    if (typeof Sortable !== 'undefined') {
        const eraSortable = new Sortable(list, {
            group: 'eras', // 제국력 그룹
            animation: 200,
            handle: '.era-banner', // 제국력 배너만 드래그 가능
            ghostClass: 'sortable-ghost',
            chosenClass: 'sortable-chosen',
            dragClass: 'sortable-drag',
            onStart: function(evt) {
                console.log('Era drag started:', evt.item.dataset.era);
            },
            onEnd: function(evt) {
                console.log('Era drag ended');
                // 제국력 순서 변경 시 데이터 재정렬 필요
                reorderEras();
            }
        });
        console.log('Era Sortable created:', eraSortable);
    }
}

// 제국력 순서 재정렬
function reorderEras() {
    const list = document.getElementById('tl-list');
    const eraGroups = list.querySelectorAll('.era-group');
    const newEraOrder = Array.from(eraGroups).map(group => group.dataset.era);
    
    // window.state.eras 순서 업데이트
    window.state.eras = newEraOrder;
    
    // 로컬 스토리지에 저장
    localStorage.setItem('storyboardEras', JSON.stringify(window.state.eras));
    
    // Firebase에 저장
    if (db && db.ref) {
        db.ref('users/Yuta/projects/기본/eras').set(window.state.eras);
    }
    
    console.log('Eras reordered:', newEraOrder);
}

// 연대 목록 렌더링: 관리 모달에 표시
function renderEras() {
    const list = document.getElementById('era-list');
    list.innerHTML = window.state.eras.map((era, idx) => `
        <div style="display: flex; justify-content: space-between; align-items: center; padding: 8px 0; border-bottom: 1px solid #eee;">
            <span style="font-size: 13px;">${era}</span>
            <div>
                <button class="f-btn-text" style="font-size: 11px;" onclick="moveEra(${idx}, -1)">▲</button>
                <button class="f-btn-text" style="font-size: 11px;" onclick="moveEra(${idx}, 1)">▼</button>
                <button class="f-btn-text" style="font-size: 11px; color: #ff6b6b;" onclick="deleteEra(${idx})">삭제</button>
            </div>
        </div>
    `).join('');
}

// 연대 이동: 위/아래로 순서 변경
function moveEra(idx, dir) {
    const newIdx = idx + dir;
    if (newIdx < 0 || newIdx >= window.state.eras.length) return;
    const temp = window.state.eras[idx];
    window.state.eras[idx] = window.state.eras[newIdx];
    window.state.eras[newIdx] = temp;
    db.ref('users/Yuta/projects/기본/settings/eras').set(window.state.eras);
}

// 연대 삭제
function deleteEra(idx) {
    window.state.eras.splice(idx, 1);
    db.ref('users/Yuta/projects/기본/settings/eras').set(window.state.eras);
}

// 새 연대 추가
function addNewEra() {
    const name = document.getElementById('new-era-name').value.trim();
    if (!name) return;
    window.state.eras.push(name);
    document.getElementById('new-era-name').value = '';
    db.ref('users/Yuta/projects/기본/settings/eras').set(window.state.eras);
}

// 이벤트 상세 모달 열기: 탭 생성 및 내용 표시
function openDetailModal(id) {
    state.currentId = String(id);
    const ev = state.storyboard.find(i => i.id === state.currentId);
    if (!ev) return;

    const container = document.getElementById('modal-tab-container');
    container.innerHTML = `<div class="top-tab active" onclick="switchDetailTab('desc', this)">내용</div>`;

    // 가지(분기)가 있으면 탭 추가
    if (ev.branches) {
        Object.values(ev.branches).forEach((b, idx) => {
            const color = `var(--p${(idx % 4) + 1})`;
            container.innerHTML += `<div class="top-tab" onclick="switchDetailTab('branch', this, '${b.id}', '${color}')">가지${idx + 1}</div>`;
        });
    }

    switchDetailTab('desc', container.firstChild);
    openModal('detailModal');
}

// 상세 탭 전환: 내용 또는 가지 표시
function switchDetailTab(type, el, bid = null, color = 'white') {
    const ev = window.state.storyboard.find(i => i.id === window.state.currentId);
    const body = document.getElementById('detail-body');
    const footer = document.getElementById('detail-footer');

    // 모든 탭에서 active 제거
    document.querySelectorAll('.top-tab').forEach(t => {
        t.classList.remove('active');
        t.style.background = 'rgba(255, 255, 255, 0.4)';
    });

    el.classList.add('active');

    if (type === 'desc') {
        el.style.background = 'white';
        body.innerText = ev.text;
    } else {
        el.style.background = color;
        body.innerText = ev.branches[bid].text;
    }

    // 푸터 버튼 설정
    if (type === 'desc') {
        footer.innerHTML = `
            <button class="f-btn-text" onclick="openEditModal()">수정</button>
            <button class="f-btn-text" onclick="openModal('branchModal')">가지추가</button>
            <button class="f-btn-text" style="color: #ff6b6b;" onclick="customConfirm('정말 삭제할까요?', deleteCurrentEvent)">삭제</button>
            <button class="f-btn-text" style="color: #adb5bd;" onclick="closeModal('detailModal')">닫기</button>
        `;
    } else {
        footer.innerHTML = `
            <button class="f-btn-text" onclick="editBranch('${bid}')">수정</button>
            <button class="f-btn-text" style="color: #ff6b6b;" onclick="customConfirm('이 가지만 삭제할까요?', () => deleteBranch('${bid}'))">삭제</button>
            <button class="f-btn-text" style="color: #adb5bd;" onclick="closeModal('detailModal')">닫기</button>
        `;
    }
}

// 현재 이벤트 삭제
function deleteCurrentEvent() {
    db.ref(`users/Yuta/projects/기본/storyboard/${window.state.currentId}`).remove().then(() => {
        closeModal('detailModal');
    });
}

// 가지 삭제
function deleteBranch(bid) {
    db.ref(`users/Yuta/projects/기본/storyboard/${window.state.currentId}/branches/${bid}`).remove().then(() => {
        closeModal('detailModal');
    });
}

// 가지 편집
function editBranch(bid) {
    const ev = window.state.storyboard.find(i => i.id === window.state.currentId);
    const newText = prompt("가지 내용 수정", ev.branches[bid].text);
    if (newText) {
        db.ref(`users/Yuta/projects/기본/storyboard/${window.state.currentId}/branches/${bid}`).update({ text: newText }).then(() => {
            closeModal('detailModal');
        });
    }
}

// 새 이벤트 추가 모달 열기
function openAddModal() {
    window.state.currentId = null;
    document.getElementById('event-title').innerText = "새로운 기록";
    document.getElementById('e-text').value = "";
    // 연대가 로드되지 않았으면 기본값 설정
    if (window.state.eras.length === 0) {
        window.state.eras = ['제국력', '대한민국'];
        updateEraSelect();
    }
    openModal('eventModal');
}

// 이벤트 편집 모달 열기
function openEditModal() {
    const ev = window.state.storyboard.find(i => i.id === window.state.currentId);
    document.getElementById('event-title').innerText = "기록 수정";
    document.getElementById('e-era').value = ev.era;
    document.getElementById('e-year').value = ev.year;
    document.getElementById('e-month').value = ev.month;
    document.getElementById('e-text').value = ev.text;
    closeModal('detailModal');
    openModal('eventModal');
}

// 이벤트 저장: 새로 추가 또는 수정
function saveEvent() {
    const id = window.state.currentId || String(Date.now());
    const data = {
        id,
        era: document.getElementById('e-era').value,
        year: document.getElementById('e-year').value,
        month: document.getElementById('e-month').value,
        text: document.getElementById('e-text').value,
        order: window.state.currentId ? window.state.storyboard.find(i => i.id === id).order : window.state.storyboard.length
    };
    db.ref(`users/Yuta/projects/기본/storyboard/${id}`).update(data).then(() => {
        closeModal('eventModal');
    });
}

// 모든 이벤트 순서 재정렬
function reorderAll() {
    const updates = {};
    document.querySelectorAll('.timeline-card').forEach((card, i) => {
        updates[`${card.dataset.id}/order`] = i;
    });
    db.ref('users/Yuta/projects/기본/storyboard').update(updates);
}

// 가지 저장
function saveBranch() {
    const bid = String(Date.now());
    db.ref(`users/Yuta/projects/기본/storyboard/${window.state.currentId}/branches/${bid}`).set({
        id: bid,
        text: document.getElementById('b-text').value
    }).then(() => {
        document.getElementById('b-text').value = "";
        closeModal('branchModal');
        setTimeout(() => openDetailModal(window.state.currentId), 100);
    });
}

// 커스텀 확인 모달
function customConfirm(msg, onOk) {
    document.getElementById('confirm-msg').innerText = msg;
    openModal('confirmModal');
    document.getElementById('confirm-ok').onclick = () => {
        onOk();
        closeModal('confirmModal');
    };
}

// 모달 열기
function openModal(id) {
    document.getElementById(id).classList.add('active');
}

// 모달 닫기
function closeModal(id) {
    document.getElementById(id).classList.remove('active');
}

// 초기 데이터 동기화 실행
syncData();