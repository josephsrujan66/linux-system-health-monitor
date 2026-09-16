/*
 * =========================================================
 * Linux System Monitor
 * Dashboard JavaScript
 * =========================================================
 */


/* =========================================================
   HELPER FUNCTIONS
   ========================================================= */

/*
 * Convert Unix timestamp to readable local time.
 */
function formatTimestamp(timestamp)
{
    if (!timestamp || timestamp === 0)
    {
        return "--";
    }

    const date = new Date(timestamp * 1000);

    return date.toLocaleTimeString();
}


/*
 * Convert large byte values into readable units.
 */
function formatBytes(bytes)
{
    if (bytes === 0)
    {
        return "0 B";
    }

    const units = [
        "B",
        "KB",
        "MB",
        "GB",
        "TB"
    ];

    let value = Number(bytes);
    let unitIndex = 0;

    while (value >= 1024 &&
           unitIndex < units.length - 1)
    {
        value /= 1024;
        unitIndex++;
    }

    return value.toFixed(2) +
           " " +
           units[unitIndex];
}


/*
 * Update a status element.
 */
function updateStatus(elementId, state)
{
    const element =
        document.getElementById(elementId);

    element.textContent =
        "● " + state;

    element.className =
        "status " +
        state.toLowerCase();
}


/*
 * Update progress bar.
 */
function updateProgress(elementId, value)
{
    const element =
        document.getElementById(elementId);

    /*
     * Keep the value between 0 and 100.
     */
    const percentage =
        Math.max(
            0,
            Math.min(100, Number(value))
        );

    element.style.width =
        percentage + "%";
}


/*
 * Update overall dashboard state.
 */
function updateOverallStatus(data)
{
    const states = [
        data.cpu.state,
        data.memory.state,
        data.disk.state,
        data.network.state
    ];

    const overallStatus =
        document.getElementById(
            "overall-status"
        );

    const overallDot =
        document.getElementById(
            "overall-dot"
        );

    /*
     * ERROR has highest priority.
     */
    if (states.includes("ERROR"))
    {
        overallStatus.textContent =
            "ERROR";

        overallDot.textContent = "●";
        overallDot.style.color = "red";

        return;
    }

    /*
     * UNKNOWN means monitoring
     * data is not ready yet.
     */
    if (states.includes("UNKNOWN"))
    {
        overallStatus.textContent =
            "UNKNOWN";

        overallDot.textContent = "●";
        overallDot.style.color = "gray";

        return;
    }

    /*
     * Everything is valid.
     */
    overallStatus.textContent =
        "HEALTHY";

    overallDot.textContent = "●";
    overallDot.style.color = "green";
}


/* =========================================================
   CPU
   ========================================================= */

function updateCPU(data)
{
    document.getElementById(
        "cpu-value"
    ).textContent =
        Number(data.cpu.usage).toFixed(2) +
        "%";

    updateStatus(
        "cpu-state",
        data.cpu.state
    );

    updateProgress(
        "cpu-progress",
        data.cpu.usage
    );

    document.getElementById(
        "cpu-updated"
    ).textContent =
        formatTimestamp(
            data.cpu.updated
        );
}


/* =========================================================
   MEMORY
   ========================================================= */

function updateMemory(data)
{
    document.getElementById(
        "memory-value"
    ).textContent =
        Number(data.memory.usage).toFixed(2) +
        "%";

    updateStatus(
        "memory-state",
        data.memory.state
    );

    updateProgress(
        "memory-progress",
        data.memory.usage
    );

    document.getElementById(
        "memory-updated"
    ).textContent =
        formatTimestamp(
            data.memory.updated
        );
}


/* =========================================================
   DISK
   ========================================================= */

function updateDisk(data)
{
    document.getElementById(
        "disk-value"
    ).textContent =
        Number(data.disk.usage).toFixed(2) +
        "%";

    updateStatus(
        "disk-state",
        data.disk.state
    );

    updateProgress(
        "disk-progress",
        data.disk.usage
    );

    document.getElementById(
        "disk-updated"
    ).textContent =
        formatTimestamp(
            data.disk.updated
        );
}


/* =========================================================
   SYSTEM INFORMATION
   ========================================================= */

function updateSystemInfo(data)
{
    document.getElementById(
        "hostname"
    ).textContent =
        data.system.hostname;

    document.getElementById(
        "os-name"
    ).textContent =
        data.system.os_name;

    document.getElementById(
        "kernel-version"
    ).textContent =
        data.system.kernel_version;

    document.getElementById(
        "architecture"
    ).textContent =
        data.system.architecture;
}


/* =========================================================
   NETWORK
   ========================================================= */

function updateNetwork(data)
{
    const network =
        data.network;

    /*
     * Summary
     */
    document.getElementById(
        "interface-count"
    ).textContent =
        network.interface_count;

    document.getElementById(
        "network-state-value"
    ).textContent =
        network.state;

    document.getElementById(
        "network-updated"
    ).textContent =
        formatTimestamp(
            network.updated
        );

    /*
     * Status
     */
    const networkStatus =
        document.getElementById(
            "network-status"
        );

    networkStatus.textContent =
        "● " + network.state;

    networkStatus.className =
        "network-status " +
        network.state.toLowerCase();

    /*
     * Table
     */
    const tableBody =
        document.getElementById(
            "network-table-body"
        );

    tableBody.innerHTML = "";

    if (network.interfaces.length === 0)
    {
        const row =
            document.createElement("tr");

        row.innerHTML =
            '<td colspan="5" class="empty-row">' +
            'No network interfaces found' +
            '</td>';

        tableBody.appendChild(row);

        return;
    }

    network.interfaces.forEach(
        function(interfaceData)
        {
            const row =
                document.createElement("tr");

            const interfaceStatus =
                interfaceData.is_up
                    ? "UP"
                    : "DOWN";

            const interfaceClass =
                interfaceData.is_up
                    ? "interface-up"
                    : "interface-down";

            row.innerHTML =
                "<td>" +
                    interfaceData.name +
                "</td>" +

                "<td>" +
                    interfaceData.ip_address +
                "</td>" +

                '<td class="' +
                    interfaceClass +
                '">' +
                    "● " +
                    interfaceStatus +
                "</td>" +

                "<td>" +
                    formatBytes(
                        interfaceData.rx_bytes
                    ) +
                "</td>" +

                "<td>" +
                    formatBytes(
                        interfaceData.tx_bytes
                    ) +
                "</td>";

            tableBody.appendChild(row);
        }
    );
}


/* =========================================================
   MAIN DASHBOARD UPDATE
   ========================================================= */

async function updateDashboard()
{
    try
    {
        const response =
            await fetch(
                "/api/status"
            );

        if (!response.ok)
        {
            throw new Error(
                "HTTP error: " +
                response.status
            );
        }

        const data =
            await response.json();

        /*
         * Update all dashboard sections.
         */
        updateSystemInfo(data);

        updateCPU(data);

        updateMemory(data);

        updateDisk(data);

        updateNetwork(data);

        updateOverallStatus(data);

        console.log(
            "Dashboard updated:",
            data
        );
    }
    catch (error)
    {
        console.error(
            "Failed to update dashboard:",
            error
        );
    }
}


/* =========================================================
   INITIAL UPDATE
   ========================================================= */

updateDashboard();


/* =========================================================
   PERIODIC UPDATE
   ========================================================= */

setInterval(
    updateDashboard,
    5000
);
