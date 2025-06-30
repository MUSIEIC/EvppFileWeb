document.addEventListener('DOMContentLoaded', () => {
    const showListBtn = document.getElementById('showListBtn');
    const searchBtn = document.getElementById('searchBtn');
    const uploadForm = document.getElementById('uploadForm');
    const fileList = document.getElementById('fileList').getElementsByTagName('tbody')[0];
    const searchInput = document.getElementById('searchInput');

    // 显示文件列表
   showListBtn.addEventListener('click', async () => {
    try {
        const host = window.location.hostname; // 获取本机 IP 或域名
        const port = 12345; // 端口号保持固定
        const url = `http://${host}:${port}/api/files`;

        const response = await fetch(url);
        if (!response.ok) throw new Error('获取文件列表失败');
        const files = await response.json();
        renderFileList(files.files);
    } catch (error) {
        console.error('显示文件列表错误:', error);
    }
});
    // 搜索文件
    searchBtn.addEventListener('click', async () => {
        const keyword = searchInput.value.trim();
        if (!keyword) return;

        try {
            const host = window.location.hostname; // 获取本机 IP 或域名
            const port = 12345; // 端口号保持固定
            const url = `http://${host}:${port}/api/files/search?keywords=${encodeURIComponent(keyword)}`;
            const response = await fetch(url);
            // const response = await fetch(`http://127.0.0.1:12345/api/files/search?keyword=${encodeURIComponent(keyword)}`);
            if (!response.ok) {
                const errorData = await response.json();
                const errorMessage = errorData.message || '删除失败';
                showError(errorMessage);
                throw new Error('搜索文件失败');
            }
            const results = await response.json();
            renderFileList(results.files);
        } catch (error) {
            console.error('搜索文件错误:', error);
        }
    });

    // 文件上传
    uploadForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const fileInput = document.getElementById('uploadFile');
        const commentInput = document.getElementById('fileComment');
        const file = fileInput.files[0];
        const comment = commentInput.value.trim();

        if (!file) return;

        const formData = new FormData();
        formData.append('file', file);
        formData.append('comment', comment);

        try {
            const host = window.location.hostname; // 获取本机 IP 或域名
            const port = 12345; // 端口号保持固定
            const url = `http://${host}:${port}/api/files/upload`;
            const response = await fetch(url, {
                method: 'POST',
                body: formData
            });
            if (!response.ok) throw new Error('文件上传失败');
            alert('文件上传成功');
            uploadForm.reset();
        } catch (error) {
            console.error('文件上传错误:', error);
        }
    });

function renderFileList(files) {
    fileList.innerHTML = '';
    files.forEach(file => {
        const row = fileList.insertRow();
        row.insertCell().textContent = file.name;
        row.insertCell().textContent = file.location;
        row.insertCell().textContent = file.path;
        row.insertCell().textContent = file.type;
        row.insertCell().textContent = file.comment;

        const actionCell = row.insertCell();

        // 下载按钮
        const downloadBtn = document.createElement('button');
        downloadBtn.textContent = '下载';
        const host = window.location.hostname;
        const port = 12345;
        const url = `http://${host}:${port}/api/files/download?path=${encodeURIComponent(file.path)}`;
        downloadBtn.addEventListener('click', () => {
            window.open(url, '_blank');
        });
        actionCell.appendChild(downloadBtn);

        // 删除按钮
        const deleteBtn = document.createElement('button');
        deleteBtn.textContent = '删除';
        deleteBtn.style.marginLeft = '10px';
        deleteBtn.addEventListener('click', async () => {
            if (!confirm('确定要删除该文件吗？')) return; // 弹窗确认

            try {
                const response = await fetch(`http://${host}:${port}/api/files/delete?path=${encodeURIComponent(file.path)}`, {
                    method: 'POST'
                });

                if (!response.ok) {
                    const errorData = await response.json();
                    const errorMessage = errorData.message || '删除失败';
                    showError(errorMessage);
                    return;
                }

                // 刷新文件列表
                showListBtn.click(); // 触发显示列表按钮的点击事件
            } catch (error) {
                console.error('删除文件错误:', error);
                showError('无法连接到服务器或发生未知错误');
            }
        });

        actionCell.appendChild(deleteBtn);
    });
}

// 显示错误信息的函数
function showError(message) {
    const errorDiv = document.getElementById('error-message');
    errorDiv.textContent = message;
    errorDiv.style.display = 'block';
}

// 创建错误信息显示区域（如果尚未存在）
const errorMessage = document.createElement('div');
errorMessage.id = 'error-message';
errorMessage.style.color = 'red';
errorMessage.style.marginTop = '10px';
errorMessage.style.display = 'none';

// 获取文件列表容器
const fileListContainer = document.getElementById('fileList');
if (fileListContainer) {
    // 将错误信息 div 插入到文件列表容器前面
    fileListContainer.parentNode.insertBefore(errorMessage, fileListContainer);
} else {
    // 如果没有找到文件列表容器，则添加到 body 末尾
    document.body.appendChild(errorMessage);
}

});